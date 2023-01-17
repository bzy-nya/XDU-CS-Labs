#include "ix_index_handle.h"

#include "ix_scan.h"

IxIndexHandle::IxIndexHandle(DiskManager *disk_manager, BufferPoolManager *buffer_pool_manager, int fd)
    : disk_manager_(disk_manager), buffer_pool_manager_(buffer_pool_manager), fd_(fd) {
    // init file_hdr_
    disk_manager_->read_page(fd, IX_FILE_HDR_PAGE, (char *)&file_hdr_, sizeof(file_hdr_));
    // disk_manager管理的fd对应的文件中，设置从原来编号+1开始分配page_no
    disk_manager_->set_fd2pageno(fd, disk_manager_->get_fd2pageno(fd) + 1);
}

/**
 * @brief 用于查找指定键所在的叶子结点
 *
 * @param key 要查找的目标key值
 * @param operation 查找到目标键值对后要进行的操作类型
 * @param transaction 事务参数，如果不需要则默认传入nullptr
 * @return 返回目标叶子结点
 * @note need to Unpin the leaf node outside!
 */
IxNodeHandle *IxIndexHandle::FindLeafPage(const char *key, Operation operation, Transaction *transaction) {
    //if( operation == Operation::INSERT ) { std::cout << "[Index] Insert key" << " " << *(int32_t *)key << "\n"; }
    //if( operation == Operation::FIND ) { std::cout << "[Index] Find key" << " " << *(int32_t *)key << "\n"; }
    //if( operation == Operation::DELETE ) { std::cout << "[Index] Delete key" << " " << *(int32_t *)key << "\n"; }
    IxNodeHandle *node = FetchNode(file_hdr_.root_page);
    assert( node != nullptr );
    while( !node -> IsLeafPage() ) {
        node = FetchNode(node -> InternalLookup(key));
        buffer_pool_manager_ -> UnpinPage(node -> GetPageId(), false);
    }
    return node;
}

/**
 * @brief 用于查找指定键在叶子结点中的对应的值result
 *
 * @param key 查找的目标key值
 * @param result 用于存放结果的容器
 * @param transaction 事务指针
 * @return bool 返回目标键值对是否存在
 */
bool IxIndexHandle::GetValue(const char *key, std::vector<Rid> *result, Transaction *transaction) {
    std::scoped_lock lock{root_latch_};
    auto leaf = FindLeafPage(key, Operation::FIND, transaction);
    
    Rid *rid;
    bool flag = leaf -> LeafLookup(key, &rid);
    if( flag ) result -> push_back(*rid);

    buffer_pool_manager_ -> UnpinPage(leaf -> GetPageId(), false);
    return flag;
}

/**
 * @brief 将指定键值对插入到B+树中
 *
 * @param (key, value) 要插入的键值对
 * @param transaction 事务指针
 * @return 是否插入成功
 */
bool IxIndexHandle::insert_entry(const char *key, const Rid &value, Transaction *transaction) {
    std::scoped_lock lock{root_latch_};
    auto leaf = FindLeafPage(key, Operation::INSERT, transaction);

    auto pre = leaf -> GetSize();
    bool flag = ( leaf -> Insert(key, value) != pre );

    if( leaf -> GetSize() == leaf -> GetMaxSize() ) {
        auto bro = Split(leaf);
        InsertIntoParent( leaf, bro -> get_key(0), bro, transaction );
        buffer_pool_manager_ -> UnpinPage( bro -> GetPageId(), true );
    }

    buffer_pool_manager_ -> UnpinPage( leaf -> GetPageId(), flag);
    return flag;
}

void IxIndexHandle::InitNode(IxNodeHandle *node) {
    node -> page_hdr -> is_leaf = false;
    node -> page_hdr -> next_free_page_no = IX_NO_PAGE;
    node -> page_hdr -> next_leaf = IX_NO_PAGE;
    node -> page_hdr -> prev_leaf = IX_NO_PAGE;
    node -> page_hdr -> num_key = 0;
    node -> page_hdr -> parent = IX_NO_PAGE;
}

/**
 * @brief 将传入的一个node拆分(Split)成两个结点，在node的右边生成一个新结点new node
 *
 * @param node 需要拆分的结点
 * @return 拆分得到的new_node
 * @note 本函数执行完毕后，原node和new node都需要在函数外面进行unpin
 */
IxNodeHandle *IxIndexHandle::Split(IxNodeHandle *node) {
    auto bro = CreateNode(); InitNode(bro);

    //std::cout << "[Index] node<" << node -> GetPageNo() << "> split and create new node<" << bro -> GetPageNo() << ">\n";
    
    if( node -> IsLeafPage() ) {
        bro -> page_hdr -> is_leaf = true;
        
        auto nxt = FetchNode( node -> GetNextLeaf() );
        if( node -> GetPageNo() == file_hdr_.last_leaf ) {
            file_hdr_.last_leaf = bro -> GetPageNo();
        }

        bro -> SetNextLeaf( node -> GetNextLeaf() );
        nxt -> SetPrevLeaf( bro -> GetPageNo() );

        bro -> SetPrevLeaf( node -> GetPageNo() );
        node -> SetNextLeaf( bro -> GetPageNo() );

        buffer_pool_manager_ -> UnpinPage(nxt -> GetPageId(), true);
    }

    int pos = node -> GetSize() / 2;
    bro -> insert_pairs(0, node -> get_key(pos), node -> get_rid(pos), (node -> GetSize() + 1) / 2);
    node -> SetSize(pos);
    for( int i = 0; i < bro -> GetSize(); i ++ ) maintain_child(bro, i);

    return bro;
}

/**
 * @brief Insert key & value pair into internal page after split
 * 拆分(Split)后，向上找到old_node的父结点
 * 将new_node的第一个key插入到父结点，其位置在 父结点指向old_node的孩子指针 之后
 * 如果插入后>=maxsize，则必须继续拆分父结点，然后在其父结点的父结点再插入，即需要递归
 * 直到找到的old_node为根结点时，结束递归（此时将会新建一个根R，关键字为key，old_node和new_node为其孩子）
 *
 * @param (old_node, new_node) 原结点为old_node，old_node被分裂之后产生了新的右兄弟结点new_node
 * @param key 要插入parent的key
 * @note 一个结点插入了键值对之后需要分裂，分裂后左半部分的键值对保留在原结点，在参数中称为old_node，
 * 右半部分的键值对分裂为新的右兄弟节点，在参数中称为new_node（参考Split函数来理解old_node和new_node）
 * @note 本函数执行完毕后，new node和old node都需要在函数外面进行unpin
 */
void IxIndexHandle::InsertIntoParent(IxNodeHandle *old_node, const char *key, IxNodeHandle *new_node,
                                     Transaction *transaction) {
    IxNodeHandle *fa; int pos;

    if( old_node -> IsRootPage() ) {
        auto newroot = CreateNode(); InitNode(newroot);

        //std::cout << "[Index] new root node<" << newroot -> GetPageNo() << "> created\n";

        file_hdr_.root_page = newroot -> GetPageNo();
        newroot -> insert_pair(0, old_node -> get_key(0), Rid{old_node -> GetPageNo(), -1} );
        old_node -> SetParentPageNo( newroot -> GetPageNo() );
        fa = newroot; pos = 0;
    } else fa = FetchNode( old_node -> GetParentPageNo() ), pos = fa -> find_child(old_node);
    fa -> insert_pair(pos + 1, key, Rid{new_node -> GetPageNo(), -1} );
    new_node -> SetParentPageNo( fa -> GetPageNo() );

    if( fa -> GetSize() >= fa -> GetMaxSize() ) {
        auto fa_bro = Split(fa);
        InsertIntoParent(fa, fa_bro -> get_key(0), fa_bro, transaction);
        buffer_pool_manager_ -> UnpinPage( fa_bro -> GetPageId(), true );
    }

    buffer_pool_manager_ -> UnpinPage( fa -> GetPageId(), true );
}  

/**
 * @brief 用于删除B+树中含有指定key的键值对
 *
 * @param key 要删除的key值
 * @param transaction 事务指针
 * @return 是否删除成功
 */
bool IxIndexHandle::delete_entry(const char *key, Transaction *transaction) {
    std::scoped_lock lock{root_latch_};
    auto leaf = FindLeafPage(key, Operation::DELETE, transaction);

    int pre = leaf -> GetSize();
    bool flag = leaf -> Remove(key) != pre;

    maintain_parent(leaf);

    if(flag) CoalesceOrRedistribute(leaf, transaction);
    buffer_pool_manager_ -> UnpinPage(leaf -> GetPageId(), true);
    

    return flag;
}

/**
 * @brief 用于处理合并和重分配的逻辑，用于删除键值对后调用
 *
 * @param node 执行完删除操作的结点
 * @param transaction 事务指针
 * @param root_is_latched 传出参数：根节点是否上锁，用于并发操作
 * @return 是否需要删除结点
 * @note User needs to first find the sibling of input page.
 * If sibling's size + input page's size >= 2 * page's minsize, then redistribute.
 * Otherwise, merge(Coalesce).
 */
bool IxIndexHandle::CoalesceOrRedistribute(IxNodeHandle *node, Transaction *transaction) {
    if( node -> IsRootPage() ) return AdjustRoot(node);
    if( node -> GetSize() >= node -> GetMinSize() ) return false;

    auto fa  = FetchNode( node -> GetParentPageNo() );
    auto bro = FetchNode( fa -> find_child(node) ? node -> GetPrevLeaf() : node -> GetNextLeaf() );

    bool flag = node -> GetSize() + bro -> GetSize() >= 2 * node -> GetMinSize();
    if( flag ) Redistribute(bro, node, fa, fa -> find_child(node));
    else Coalesce(&bro, &node, &fa, fa -> find_child(node), transaction);

    buffer_pool_manager_ -> UnpinPage( fa -> GetPageId(), true );
    buffer_pool_manager_ -> UnpinPage( bro -> GetPageId(), true );
    return flag;
}

/**
 * @brief 用于当根结点被删除了一个键值对之后的处理
 *
 * @param old_root_node 原根节点
 * @return bool 根结点是否需要被删除
 * @note size of root page can be less than min size and this method is only called within coalesceOrRedistribute()
 */
bool IxIndexHandle::AdjustRoot(IxNodeHandle *old_root_node) {
    if( old_root_node -> IsLeafPage() ) {
        file_hdr_.root_page = old_root_node -> GetPageNo();
        return false;
    } 
    if( old_root_node -> IsRootPage() and old_root_node -> GetSize() == 1 ){
        file_hdr_.root_page = old_root_node -> ValueAt(0);

        auto new_root = FetchNode(file_hdr_.root_page);
        new_root -> page_hdr -> parent = IX_NO_PAGE;
        buffer_pool_manager_ -> UnpinPage(new_root -> GetPageId(), true);
        
        release_node_handle(*old_root_node);
        
        return true;
    }
    return false;
}

/**
 * @brief 重新分配node和兄弟结点neighbor_node的键值对
 * Redistribute key & value pairs from one page to its sibling page. If index == 0, move sibling page's first key
 * & value pair into end of input "node", otherwise move sibling page's last key & value pair into head of input "node".
 *
 * @param neighbor_node sibling page of input "node"
 * @param node input from method coalesceOrRedistribute()
 * @param parent the parent of "node" and "neighbor_node"
 * @param index node在parent中的rid_idx
 * @note node是之前刚被删除过一个key的结点
 * index=0，则neighbor是node后继结点，表示：node(left)      neighbor(right)
 * index>0，则neighbor是node前驱结点，表示：neighbor(left)  node(right)
 * 注意更新parent结点的相关kv对
 */
void IxIndexHandle::Redistribute(IxNodeHandle *neighbor_node, IxNodeHandle *node, IxNodeHandle *parent, int index) {
    int size_node = node -> GetSize();
    int size_bro  = neighbor_node -> GetSize();

    if( index ) {
        node -> insert_pair( 0, neighbor_node -> get_key(size_bro - 1), *(neighbor_node -> get_rid(size_bro - 1)) );
        neighbor_node -> erase_pair(size_bro - 1);
        parent -> set_key(parent -> find_child(node), node -> get_key(0));
        maintain_child(node, 0);
    } else {
        node -> insert_pair( size_node, neighbor_node -> get_key(0), *(neighbor_node -> get_rid(0)) );
        neighbor_node -> erase_pair(0);
        parent -> set_key(parent -> find_child(neighbor_node), neighbor_node -> get_key(0));
        maintain_child(node, size_node);
    }
}

/**
 * @brief 合并(Coalesce)函数是将node和其直接前驱进行合并，也就是和它左边的neighbor_node进行合并；
 * 假设node一定在右边。如果上层传入的index=0，说明node在左边，那么交换node和neighbor_node，保证node在右边；合并到左结点，实际上就是删除了右结点；
 * Move all the key & value pairs from one page to its sibling page, and notify buffer pool manager to delete this page.
 * Parent page must be adjusted to take info of deletion into account. Remember to deal with coalesce or redistribute
 * recursively if necessary.
 *
 * @param neighbor_node sibling page of input "node" (neighbor_node是node的前结点)
 * @param node input from method coalesceOrRedistribute() (node结点是需要被删除的)
 * @param parent parent page of input "node"
 * @param index node在parent中的rid_idx
 * @return true means parent node should be deleted, false means no deletion happend
 * @note Assume that *neighbor_node is the left sibling of *node (neighbor -> node)
 */
bool IxIndexHandle::Coalesce(IxNodeHandle **neighbor_node, IxNodeHandle **node, IxNodeHandle **parent, int index,
                             Transaction *transaction) {
    if( index == 0 ) std::swap(neighbor_node, node), index = 1;

    //std::cout << "[Index] merge node<" << (*neighbor_node) -> GetPageNo() << "> and node<" << (*node) -> GetPageNo() << ">\n";

    (*neighbor_node) -> insert_pairs( (*neighbor_node) -> GetSize(), (*node) -> keys, (*node) -> rids, (*node) -> GetSize() );

    for( int i = 1 ; i <= (*node) -> GetSize(); i ++ ) {
        maintain_child( (*neighbor_node), (*neighbor_node) -> GetSize()  - i );
    }
    if( (*node) -> IsLeafPage() ) erase_leaf(*node);
    release_node_handle(**node);

    (*parent) -> erase_pair(index);

    return CoalesceOrRedistribute(*parent);
}

/** -- 以下为辅助函数 -- */
/**
 * @brief 获取一个指定结点
 *
 * @param page_no
 * @return IxNodeHandle*
 * @note pin the page, remember to unpin it outside!
 */
IxNodeHandle *IxIndexHandle::FetchNode(int page_no) const {
    // assert(page_no < file_hdr_.num_pages); // 不再生效，由于删除操作，page_no可以大于个数
    //std::cout << "[Index] Fetch node page : " << page_no << " (" << file_hdr_.num_pages << " pages in tot)\n";
    Page *page = buffer_pool_manager_->FetchPage(PageId{fd_, page_no});
    assert( page != nullptr );
    IxNodeHandle *node = new IxNodeHandle(&file_hdr_, page);
    return node;
}

/**
 * @brief 创建一个新结点
 *
 * @return IxNodeHandle*
 * @note pin the page, remember to unpin it outside!
 * 注意：对于Index的处理是，删除某个页面后，认为该被删除的页面是free_page
 * 而first_free_page实际上就是最新被删除的页面，初始为IX_NO_PAGE
 * 在最开始插入时，一直是create node，那么first_page_no一直没变，一直是IX_NO_PAGE
 * 与Record的处理不同，Record将未插入满的记录页认为是free_page
 */
IxNodeHandle *IxIndexHandle::CreateNode() {
    file_hdr_.num_pages++;
    PageId new_page_id = {.fd = fd_, .page_no = INVALID_PAGE_ID};
    // 从3开始分配page_no，第一次分配之后，new_page_id.page_no=3，file_hdr_.num_pages=4
    Page *page = buffer_pool_manager_->NewPage(&new_page_id);
    // 注意，和Record的free_page定义不同，此处【不能】加上：file_hdr_.first_free_page_no = page->GetPageId().page_no
    IxNodeHandle *node = new IxNodeHandle(&file_hdr_, page);
    return node;
}

/**
 * @brief 从node开始更新其父节点的第一个key，一直向上更新直到根节点
 *
 * @param node
 */
void IxIndexHandle::maintain_parent(IxNodeHandle *node) {
    IxNodeHandle *curr = node;
    while (curr->GetParentPageNo() != IX_NO_PAGE) {
        // Load its parent
        IxNodeHandle *parent = FetchNode(curr->GetParentPageNo());
        int rank = parent->find_child(curr);
        char *parent_key = parent->get_key(rank);
        // char *child_max_key = curr.get_key(curr.page_hdr->num_key - 1);
        char *child_first_key = curr->get_key(0);
        if (memcmp(parent_key, child_first_key, file_hdr_.col_len) == 0) {
            assert(buffer_pool_manager_->UnpinPage(parent->GetPageId(), true));
            break;
        }
        memcpy(parent_key, child_first_key, file_hdr_.col_len);  // 修改了parent node
        curr = parent;

        assert(buffer_pool_manager_->UnpinPage(parent->GetPageId(), true));
    }
}

/**
 * @brief 要删除leaf之前调用此函数，更新leaf前驱结点的next指针和后继结点的prev指针
 *
 * @param leaf 要删除的leaf
 */
void IxIndexHandle::erase_leaf(IxNodeHandle *leaf) {
    assert(leaf->IsLeafPage());

    if( file_hdr_.last_leaf == leaf -> GetPageNo() ) {
        file_hdr_.last_leaf = leaf -> GetPrevLeaf();
    }

    IxNodeHandle *prev = FetchNode(leaf->GetPrevLeaf());
    prev->SetNextLeaf(leaf->GetNextLeaf());
    buffer_pool_manager_->UnpinPage(prev->GetPageId(), true);

    IxNodeHandle *next = FetchNode(leaf->GetNextLeaf());
    next->SetPrevLeaf(leaf->GetPrevLeaf());  // 注意此处是SetPrevLeaf()
    buffer_pool_manager_->UnpinPage(next->GetPageId(), true);
}

/**
 * @brief 删除node时，更新file_hdr_.num_pages
 *
 * @param node
 */
void IxIndexHandle::release_node_handle(IxNodeHandle &node) { file_hdr_.num_pages--; }

/**
 * @brief 将node的第child_idx个孩子结点的父节点置为node
 */
void IxIndexHandle::maintain_child(IxNodeHandle *node, int child_idx) {
    if (!node->IsLeafPage()) {
        //  Current node is inner node, load its child and set its parent to current node
        int child_page_no = node->ValueAt(child_idx);
        IxNodeHandle *child = FetchNode(child_page_no);
        child->SetParentPageNo(node->GetPageNo());
        buffer_pool_manager_->UnpinPage(child->GetPageId(), true);
    }
}

/**
 * @brief 这里把iid转换成了rid，即iid的slot_no作为node的rid_idx(key_idx)
 * node其实就是把slot_no作为键值对数组的下标
 * 换而言之，每个iid对应的索引槽存了一对(key,rid)，指向了(要建立索引的属性首地址,插入/删除记录的位置)
 *
 * @param iid
 * @return Rid
 * @note iid和rid存的不是一个东西，rid是上层传过来的记录位置，iid是索引内部生成的索引槽位置
 */
Rid IxIndexHandle::get_rid(const Iid &iid) const {
    IxNodeHandle *node = FetchNode(iid.page_no);
    if (iid.slot_no >= node->GetSize()) {
        throw IndexEntryNotFoundError();
    }
    buffer_pool_manager_->UnpinPage(node->GetPageId(), false);  // unpin it!
    return *node->get_rid(iid.slot_no);
}

/** --以下函数将用于lab3执行层-- */
/**
 * @brief FindLeafPage + lower_bound
 *
 * @param key
 * @return Iid
 * @note 上层传入的key本来是int类型，通过(const char *)&key进行了转换
 * 可用*(int *)key转换回去
 */
Iid IxIndexHandle::lower_bound(const char *key) {
    // int int_key = *(int *)key;
    // printf("my_lower_bound key=%d\n", int_key);

    IxNodeHandle *node = FindLeafPage(key, Operation::FIND, nullptr);
    int key_idx = node->lower_bound(key);

    Iid iid = {.page_no = node->GetPageNo(), .slot_no = key_idx};

    // unpin leaf node
    buffer_pool_manager_->UnpinPage(node->GetPageId(), false);
    return iid;
}

/**
 * @brief FindLeafPage + upper_bound
 *
 * @param key
 * @return Iid
 */
Iid IxIndexHandle::upper_bound(const char *key) {
    // int int_key = *(int *)key;
    // printf("my_upper_bound key=%d\n", int_key);

    IxNodeHandle *node = FindLeafPage(key, Operation::FIND, nullptr);
    int key_idx = node->upper_bound(key);

    Iid iid;
    if (key_idx == node->GetSize()) {
        // 这种情况无法根据iid找到rid，即后续无法调用ih->get_rid(iid)
        iid = leaf_end();
    } else {
        iid = {.page_no = node->GetPageNo(), .slot_no = key_idx};
    }

    // unpin leaf node
    buffer_pool_manager_->UnpinPage(node->GetPageId(), false);
    return iid;
}

/**
 * @brief 指向第一个叶子的第一个结点
 * 用处在于可以作为IxScan的第一个
 *
 * @return Iid
 */
Iid IxIndexHandle::leaf_begin() const {
    Iid iid = {.page_no = file_hdr_.first_leaf, .slot_no = 0};
    return iid;
}

/**
 * @brief 指向最后一个叶子的最后一个结点的后一个
 * 用处在于可以作为IxScan的最后一个
 *
 * @return Iid
 */
Iid IxIndexHandle::leaf_end() const {
    IxNodeHandle *node = FetchNode(file_hdr_.last_leaf);
    Iid iid = {.page_no = file_hdr_.last_leaf, .slot_no = node->GetSize()};
    buffer_pool_manager_->UnpinPage(node->GetPageId(), false);  // unpin it!
    return iid;
}
