# rucbase 实验报告

目录

- 1 测试以及结果

- 2 Lab1 代码

- 3 Lab2 代码

## 1 测试以及结果

### 1.1 测试命令

在项目的根目录下编写了一个 Makefile 用于调用 Rucbase Lab 的单元测试。

```makefile
# ./Makefile
copy:
    cd build; \
    cmake .. -DCMAKE_BUILD_TYPE=Debug

t1_1: copy
    cd build; \
    make disk_manager_test; \
    ./bin/disk_manager_test

t1_2: copy
    cd build; \
    make lru_replacer_test; \
    ./bin/lru_replacer_test

t1_3: copy
    cd build; \
    make buffer_pool_manager_test; \
    ./bin/buffer_pool_manager_test

t1_4: copy
    cd build; \
    make rm_gtest; \
    ./bin/rm_gtest

t2_1: copy
    cd build; \
    make b_plus_tree_insert_test; \
    ./bin/b_plus_tree_insert_test

t2_2: copy
    cd build; \
    make b_plus_tree_delete_test; \
    ./bin/b_plus_tree_delete_test

t2_3: copy
    cd build; \
    make b_plus_tree_concurrent_test; \
    ./bin/b_plus_tree_concurrent_test

all: t1_1 t1_2 t1_3 t1_4 t2_1 t2_2 t2_3

clean:
    rm -rf build
```

其中 `t1_1` 到 `t1_4` 为 Lab1 的单元测试，其中 `t2_1` 到 `t2_3` 为 Lab2 的单元测试。

### 1.2 测试结果

- `make t1_1`

![](images/2023-01-14 14-59-19.png)

- `make t1_2`

![](images/2023-01-14 14-59-54.png)

- `make t1_3`

![](images/2023-01-14 15-00-33.png)

- `make t1_4`

![](images/2023-01-14 15-01-08.png)

- `make t2_1`

![](images/2023-01-14 15-01-44.png)

- `make t2_2`

![](images/2023-01-14 15-02-29.png)

- `make t2_3`

![](images/2023-01-14 15-02-59.png)

## 2 Lab1 代码

### 2.1 storage/disk_manager.cpp

storage/disk_manager.cpp 文件用于实现 storage/disk_manager.h 定义的 `DiskManager` 类的接口，DiskManager 用于整个项目的统一文件管理，以下是实现代码。

- `DiskManager::write_page`
  
  ```cpp
  void DiskManager::write_page(int fd, page_id_t page_no, const char *offset, int num_bytes) {
      if( fd2path_.find(fd) == fd2path_.end() ) {
          throw FileNotOpenError(fd);
      }
      lseek(fd, page_no * PAGE_SIZE, SEEK_SET);
      if( write(fd, (void *)offset, num_bytes) != num_bytes ) {
          throw UnixError();
      }
  }
  ```

- `DiskManager::read_page`
  
  ```cpp
  void DiskManager::read_page(int fd, page_id_t page_no, char *offset, int num_bytes) {
      if( fd2path_.find(fd) == fd2path_.end() ) {
          throw FileNotOpenError(fd);
      }
      lseek(fd, page_no * PAGE_SIZE, SEEK_SET);
      if( read(fd, (void *)offset, num_bytes) < 0 ) {
          throw UnixError();
      }
  }
  ```

- `DiskManager::AllocatePage`
  
  ```cpp
  page_id_t DiskManager::AllocatePage(int fd) {
      return fd2pageno_[fd] ++; //
  }
  ```

- `DiskManager::is_file`
  
  ```cpp
  bool DiskManager::is_file(const std::string &path) {
      struct stat st;
      return stat(path.c_str(), &st) == 0 && (S_IFREG ==(st.st_mode & S_IFMT));
  }
  ```

- `DiskManager::create_file`
  
  ```cpp
  void DiskManager::create_file(const std::string &path) {
      if(!is_file(path)) {
          auto fd = open( path.c_str(), O_CREAT | O_RDWR, 0x1ff );
          close(fd);
      } else {
          throw FileExistsError(path);
      }
  }
  ```

- `DiskManager::destroy_file`
  
  ```cpp
  void DiskManager::destroy_file(const std::string &path) {
      if( !is_file(path) ) {
          throw FileNotFoundError(path);
      }
      if( path2fd_.find(path) == path2fd_.end() ) {
          if( unlink(path.c_str()) < 0 ) {
              throw UnixError();
          }
      } else {
          throw FileNotClosedError(path);
      }
  }
  ```

- `DiskManager::close_file`
  
  ```cpp
  void DiskManager::close_file(int fd) {
      auto it = fd2path_.find(fd);
      if( it != fd2path_.end() ) {
          close(fd);
          path2fd_.erase( path2fd_.find((*it).second) );
          fd2path_.erase(it);
      } else {
          throw FileNotOpenError(fd);
      }
  }
  ```

### 2.2 replacer/lru_replacer.cpp

replacer/lru_replacer.cpp 文件用于实现 replacer/lru_replacer.h 定义的 LRUReplacer 类的接口(继承于 replacer/replacer.h 中定义的 `Replacer` 类，`Replacer` 类用于实现 storage/buffer_pool_manager.h 中定义的 `BufferPoolManager` 类的缓存替换算法)，以下是实现代码。

- `LRUReplacer::Victim` 用于寻找替换页
  
  ```cpp
  bool LRUReplacer::Victim(frame_id_t *frame_id) {
      // C++17 std::scoped_lock
      // 它能够避免死锁发生，其构造函数能够自动进行上锁操作，析构函数会对互斥量进行解锁操作，保证线程安全。
      std::scoped_lock lock{latch_};
  
      if( LRUlist_.size() == 0 ) return false;
  
      LRUhash_.erase( LRUhash_.find( (*frame_id) = LRUlist_.back() ) );
      LRUlist_.pop_back();
      return true;
  }
  ```

- `LRUReplacer::Pin` 锁定正在操作的页
  
  ```cpp
  void LRUReplacer::Pin(frame_id_t frame_id) {
      std::scoped_lock lock{latch_};
  
      auto it = LRUhash_.find(frame_id);
      if( it != LRUhash_.end() ) {
          LRUlist_.erase( (*it).second );
          LRUhash_.erase( it ); 
  ```

- `LRUReplacer::Unpin` 释放结束操作的页
  
  ```cpp
  void LRUReplacer::Pin(frame_id_t frame_id) {
      std::scoped_lock lock{latch_};
  
      auto it = LRUhash_.find(frame_id);
      if( it != LRUhash_.end() ) {
          LRUlist_.erase( (*it).second );
          LRUhash_.erase( it ); 
      }
  }
  ```

### 2.3  storage/buffer_pool_manager.cpp

storage/buffer_pool_manager.cpp 用于实现 storage/buffer_pool_manager.h 中定义的 `BufferPoolManager` 类的接口。`BufferPoolManager` 类用于给 record/rm_file_handle.h 中定义的 `RmFileHandle` 记录文件管理类和 index/ix_index_handle.h 中定义的 `IxIndexHandle` 索引文件管理类提供统一的文件和缓存管理功能，以下是代码实现。

- `BufferPoolManager::FindVictimPage` 辅助函数，用于寻找替换页
  
  ```cpp
  bool BufferPoolManager::FindVictimPage(frame_id_t *frame_id) {
      if( free_list_.size() != 0 ) {
          (*frame_id) = free_list_.front();
          free_list_.pop_front();
          //std::cout << "alloc new frame " << *frame_id << "]]]]]\n";
          return true;
      }
      return replacer_ -> Victim( frame_id );
  }
  ```

- `BufferPoolManager::UpdatePage` 这里修改了 storage/buffer_pool_manager.h 中的默认定义，用于写入脏页以及加载替换后的新页。
  
  ```cpp
  void BufferPoolManager::UpdatePage(PageId new_page_id, frame_id_t frame_id) {
      Page *page = &pages_[frame_id];
  
      //std::cout << "[Buffer] Update: frame " << frame_id << " (" << page -> id_.fd << ", " << page -> id_.page_no <<
      //    ") -> (" << new_page_id.fd << ", " << new_page_id.page_no << ")\n";
  
      if( page -> IsDirty() ) {
          disk_manager_ -> write_page(page -> GetPageId().fd, page -> GetPageId().page_no, page -> GetData(), PAGE_SIZE);
          page->is_dirty_ = false;
      }
      page -> ResetMemory();
  
      auto it = page_table_.find(page -> id_);
      if( it != page_table_.end() ) page_table_.erase(it);
      page_table_[page -> id_ = new_page_id] = frame_id;
  
      if( page -> id_ .page_no != INVALID_FRAME_ID ) {
          disk_manager_ -> read_page(page -> GetPageId().fd, page -> GetPageId().page_no, page -> GetData(), PAGE_SIZE);
      }
  }
  ```

- `BufferPoolManager::FetchPage` 将指定页拉取到缓存中，并且锁定返回指针用于使用。
  
  ```cpp
  Page *BufferPoolManager::FetchPage(PageId page_id) {
      std::scoped_lock lock{latch_};
  
      frame_id_t frame_id;
  
      if( page_table_.find(page_id) != page_table_.end() ) {
          //std::cout << "[Buffer] Cache hit! fetch {" << page_id.fd << ", " << page_id.page_no << "}\n";
          frame_id = page_table_[page_id];
      } else {
          //std::cout << "[Buffer] Cache miss! Try to fetch page (" << page_id.fd << ", " << page_id.page_no << ") from disk\n"; 
          if( !FindVictimPage(&frame_id) ) return nullptr;
          UpdatePage(page_id, frame_id);
      }
  
      replacer_ -> Pin(frame_id);
      pages_[frame_id].pin_count_ ++;
  
      return &pages_[frame_id];
  }
  ```

- `BufferPoolManager::NewPage` 在指定文件中新建页，并将其拉取到缓存中。
  
  ```cpp
  Page *BufferPoolManager::NewPage(PageId *page_id) {
      std::scoped_lock lock{latch_};
  
      frame_id_t frame_id;
      if( !FindVictimPage(&frame_id) ) return nullptr;
  
      page_id -> page_no = disk_manager_ -> AllocatePage( page_id -> fd );
  
      UpdatePage(*page_id, frame_id);
  
      replacer_ -> Pin(frame_id);
      pages_[frame_id].pin_count_ ++;
  
      return &pages_[frame_id];
  }
  ```

- `BufferPoolManager::DeletePage` 删除指定页，不过目前 DiskManger 没有回收功能。
  
  ```cpp
  bool BufferPoolManager::DeletePage(PageId page_id) {
      std::scoped_lock lock{latch_};
  
      if( page_table_.find(page_id) == page_table_.end() ) return true;
      frame_id_t frame_id = page_table_[page_id];
  
      Page *page = &pages_[frame_id];
  
      if( page -> pin_count_ > 0 ) return false;
  
      disk_manager_->DeallocatePage(page_id.page_no);
  
      page_id.page_no =  INVALID_PAGE_ID;
      UpdatePage(page_id, frame_id);
      free_list_.push_back(frame_id);
  
      return true;
  }
  ```

### 2.4 record/rm_file_handle.cpp

record/rm_file_handle.cpp 用于实现 record/rm_file_handle.h 中实现的记录文件管理类 RmFileHandle 类，以下是实现。

- `RmFileHandle::fetch_page_handle` 辅助函数，拉取页面并返回页面的记录页面管理对象 RmPageHandle。
  
  ```cpp
  RmPageHandle RmFileHandle::fetch_page_handle(int page_no) const {
      if( page_no >= file_hdr_.num_pages ) {
          throw PageNotExistError(disk_manager_ -> GetFileName(fd_), page_no);
      }
      return RmPageHandle(&file_hdr_, buffer_pool_manager_ -> FetchPage( {fd_, page_no} ));
  }
  ```

- `RmFileHandle::create_new_page_handle` 辅助函数，新建记录页面
  
  ```cpp
  RmPageHandle RmFileHandle::create_new_page_handle() {
      PageId pageid = {fd_, INVALID_FRAME_ID};
      Page* page = buffer_pool_manager_ -> NewPage(&pageid);
  
      auto pageHandler = RmPageHandle(&file_hdr_, page);
  
      if( page != nullptr ) {
          file_hdr_.num_pages ++;
          file_hdr_.first_free_page_no = pageid.page_no;
  
          pageHandler.page_hdr -> next_free_page_no = RM_NO_PAGE;
          pageHandler.page_hdr -> num_records = 0;
          Bitmap::init(pageHandler.bitmap, file_hdr_.bitmap_size);
      }
  
      return pageHandler;
  }
  ```

- `RmFileHandle::create_page_handle` 辅助函数，新建或获取记录页面。
  
  ```cpp
  RmPageHandle RmFileHandle::create_page_handle() {
      if(file_hdr_.first_free_page_no == RM_NO_PAGE) {
          return create_new_page_handle();
      }
      return fetch_page_handle(file_hdr_.first_free_page_no);
  }
  ```

- `RmFileHandle::release_page_handle` 辅助函数，释放空的记录页面。
  
  ```cpp
  void RmFileHandle::release_page_handle(RmPageHandle &page_handle) {
      page_handle.page_hdr -> next_free_page_no = file_hdr_.first_free_page_no;
      file_hdr_.first_free_page_no = page_handle.page -> GetPageId().page_no;
  }
  ```

- `RmFileHandle::get_record`
  
  ```cpp
  std::unique_ptr<RmRecord> RmFileHandle::get_record(const Rid &rid, Context *context) const {
      auto record = std::make_unique<RmRecord>(file_hdr_.record_size);
      auto pageHandler = fetch_page_handle(rid.page_no);
  
      if( !Bitmap::is_set(pageHandler.bitmap, rid.slot_no) ) {
          throw RecordNotFoundError(rid.page_no, rid.slot_no);
      }
  
      record -> size = file_hdr_.record_size;
      memcpy( record -> data, pageHandler.get_slot(rid.slot_no), file_hdr_.record_size );
      return record;
  }
  ```

- `RmFileHandle::insert_record`
  
  ```cpp
  Rid RmFileHandle::insert_record(char *buf, Context *context) {
      auto pageHandler = create_page_handle();
      int slot_no = Bitmap::first_bit(false, pageHandler.bitmap, file_hdr_.num_records_per_page);
  
      memcpy( pageHandler.get_slot(slot_no), buf, file_hdr_.record_size );
      Bitmap::set(pageHandler.bitmap, slot_no);
  
      if( ++ pageHandler.page_hdr -> num_records == file_hdr_.num_records_per_page ) {
          file_hdr_.first_free_page_no = pageHandler.page_hdr -> next_free_page_no;
      }
  
      //std::cout << "insert recoder {" << pageHandler.page -> GetPageId().page_no << " " << slot_no << "}\n";
  
      return Rid{pageHandler.page -> GetPageId().page_no, slot_no};
  }
  ```

- `RmFileHandle::delete_record`
  
  ```cpp
  void RmFileHandle::delete_record(const Rid &rid, Context *context) {
  
      //std::cout << "delete recoder {" << rid.page_no << " " << rid.slot_no << "}\n";
  
      auto pageHandler = fetch_page_handle(rid.page_no);
  
      if( !Bitmap::is_set(pageHandler.bitmap, rid.slot_no) ) {
          throw RecordNotFoundError(rid.page_no, rid.slot_no);
      }
  
      Bitmap::reset(pageHandler.bitmap, rid.slot_no);
      if( (pageHandler.page_hdr -> num_records ) -- == file_hdr_.num_records_per_page ) {
          release_page_handle(pageHandler);
      }
  }
  ```

- `RmFileHandle::update_record`
  
  ```cpp
  void RmFileHandle::update_record(const Rid &rid, char *buf, Context *context) {
      //std::cout << "update recoder {" << rid.page_no << " " << rid.slot_no << "}\n";
      auto pageHandler = fetch_page_handle(rid.page_no);
  
      if( !Bitmap::is_set(pageHandler.bitmap, rid.slot_no) ) {
          throw RecordNotFoundError(rid.page_no, rid.slot_no);
      }
      memcpy( pageHandler.get_slot(rid.slot_no), buf, file_hdr_.record_size );
  }
  ```

### 2.5 record/rm_scan.cpp

record/rm_scan.cpp 实现 record/rm_scan.h 中定义的 RmScan 迭代器类，用于遍历 RmFileHandle 对象内部的记录。以下是核心代码实现。

- `RmScan::next`
  
  ```cpp
  void RmScan::next() {
      while( rid_.page_no < file_handle_ -> file_hdr_.num_pages ) {
          auto page_handler = file_handle_ -> fetch_page_handle(rid_.page_no);
          rid_.slot_no = Bitmap::next_bit( true, page_handler.bitmap, file_handle_->file_hdr_.num_records_per_page, rid_.slot_no );
  
          if( rid_.slot_no < file_handle_ -> file_hdr_.num_records_per_page) {
              //std::cout << "{" << rid_.page_no << " " << rid_.slot_no << "}" << file_handle_->file_hdr_.num_pages << " " 
              //    << file_handle_->file_hdr_.num_records_per_page << " " << file_handle_->file_hdr_.bitmap_size <<"\n";
              return;
          }
          rid_ = Rid{ rid_.page_no + 1, -1 };
          if( rid_.page_no >= file_handle_ -> file_hdr_.num_pages ) {
              rid_ = Rid{RM_NO_PAGE, -1};
              break;
          }
      }
  }
  ```

## 3 Lab2 代码

### 2.1 index/ix_node_handle.cpp

index/ix_node_handle.cpp 用于实现 index/ix_node_handle.h 中定义的 IxNodeHandle 类，作为 index/ix_index_handle.h 定义的 B+ Tree 索引文件管理对象 IxIndexHandle 的节点管理器。代码如下。

- `IxNodeHandle::lower_bound` 辅助函数
  
  ```cpp
  int IxNodeHandle::lower_bound(const char *target) const {
      int l = 0, r = page_hdr -> num_key;
      while( l ^ r ) {
          int mid = (l + r) >> 1;
          if( mid == r ) mid = l;
          if( ix_compare( target, get_key(mid), file_hdr -> col_type, file_hdr -> col_len) > 0 ) {
              l = mid + 1;
          } else r = mid;
      }
      return l;
  }
  ```

- `IxNodeHandle::upper_bound` 辅助函数
  
  ```cpp
  int IxNodeHandle::upper_bound(const char *target) const {
      int l = 1, r = page_hdr -> num_key;
      while( l ^ r ) {
          int mid = (l + r) >> 1;
          if( mid == r ) mid = l;
          if( ix_compare( target, get_key(mid), file_hdr -> col_type, file_hdr -> col_len) >= 0 ) {
              l = mid + 1;
          } else r = mid;
      }
      return l;
  }
  ```

- `IxNodeHandle::LeafLookup` 叶节点寻找键
  
  ```cpp
  bool IxNodeHandle::LeafLookup(const char *key, Rid **value) {
      int key_location = lower_bound(key);
      if( key_location == GetSize() or ix_compare( key, get_key(key_location), file_hdr -> col_type, file_hdr -> col_len) != 0 ) return false;
      *value = get_rid(key_location);
      return true;
  }
  ```

- `IxNodeHandle::InternalLookup` 内部节点寻找键
  
  ```cpp
  page_id_t IxNodeHandle::InternalLookup(const char *key) {
      return ValueAt(upper_bound(key) - 1);
  }
  ```

- `IxNodeHandle::insert_pairs`
  
  ```cpp
  void IxNodeHandle::insert_pairs(int pos, const char *key, const Rid *rid, int n) {
      //std::cout << "[Index-node " << GetPageNo() << "] insert " << n << " items at pos " << pos 
      //    << "(size = " << GetSize() << ", maxsize = " << GetMaxSize() << ")\n";
      int all = GetSize();
      assert( pos >= 0 and pos <= GetSize() );
  
      for( int i = all - 1; i >= pos; i -- ) {
          set_key(i + n, get_key(i));
          set_rid(i + n, *get_rid(i));
      }
      for( int i = 0; i < n; i ++ ) {
          set_key(pos + i, key + file_hdr -> col_len * i );
          set_rid(pos + i, rid[i]);
      }
      SetSize( all + n );
  }
  
  ```

- `IxNodeHandle::Insert`
  
  ```cpp
  int IxNodeHandle::Insert(const char *key, const Rid &value) {
      int location = lower_bound(key);
      if( ix_compare( key, get_key(location), file_hdr -> col_type, file_hdr -> col_len ) != 0 ) {
          insert_pair( location, key, value );
      }
      return GetSize();
  }
  ```

- `IxNodeHandle::erase_pair`
  
  ```cpp
  void IxNodeHandle::erase_pair(int pos) {
      //std::cout << "[Index-node" << GetPageNo() << "] remove key " << *(int*)get_key(pos) << " at pos " << pos << "]\n";
      assert(pos < GetSize() && pos >=0);
  
      for( int i = pos; i < GetSize(); i ++ ) {
          set_key(i, get_key(i + 1));
          set_rid(i, *get_rid(i + 1));
      }
  
      SetSize(GetSize() - 1);
  }
  ```

- `IxNodeHandle::Remove`
  
  ```cpp
  int IxNodeHandle::Remove(const char *key) { 
      int location = lower_bound(key);
      if( ix_compare( key, get_key(location), file_hdr -> col_type, file_hdr -> col_len ) == 0 ) {
          erase_pair( location );
      }
      return GetSize();
  }
  ```

### 2.2 index/ix_index_handle.cpp

index/ix_index_handle.cpp 用于实现 index/ix_index_handle.h 中定义的 B+ Tree 索引文件管理类 IxIndexHandle。以下是代码实现。

- `IxIndexHandle::InitNode` 自行添加的辅助函数，用于初始化节点。
  
  ```cpp
  void IxIndexHandle::InitNode(IxNodeHandle *node) {
      node -> page_hdr -> is_leaf = false;
      node -> page_hdr -> next_free_page_no = IX_NO_PAGE;
      node -> page_hdr -> next_leaf = IX_NO_PAGE;
      node -> page_hdr -> prev_leaf = IX_NO_PAGE;
      node -> page_hdr -> num_key = 0;
      node -> page_hdr -> parent = IX_NO_PAGE;
  }
  ```
  
  

- `IxIndexHandle::FindLeafPage` 寻找键所在叶节点，这里使用树的全局锁用于并发。
  
  ```cpp
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
  ```

- `IxIndexHandle::GetValue` 
  
  ```cpp
  bool IxIndexHandle::GetValue(const char *key, std::vector<Rid> *result, Transaction *transaction) {
      std::scoped_lock lock{root_latch_};
      auto leaf = FindLeafPage(key, Operation::FIND, transaction);
      
      Rid *rid;
      bool flag = leaf -> LeafLookup(key, &rid);
      if( flag ) result -> push_back(*rid);
  
      buffer_pool_manager_ -> UnpinPage(leaf -> GetPageId(), false);
      return flag;
  }
  ```

- `IxIndexHandle::insert_entry` 插入键值对
  
  ```cpp
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
  ```

- `IxIndexHandle::Split` 插入的辅助函数，用于分裂过载节点。
  
  ```cpp
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
  ```

- `IxIndexHandle::InsertIntoParent` 插入的辅助函数，用于将分裂后过载节点插入父节点，并且递归判断是否过载。
  
  ```cpp
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
  ```

-  `IxIndexHandle::delete_entry` 删除键
  
  ```cpp
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
  ```

- `IxIndexHandle::CoalesceOrRedistribute` 删除键的辅助函数，用于判断键不足的节点是否需要合并重分配或者修改根。
  
  ```cpp
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
  ```

- `IxIndexHandle::Redistribute` 删除键的辅助函数，重分配节点。
  
  ```cpp
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
  ```

- `IxIndexHandle::Coalesce` 删除键的辅助函数，合并节点。
  
  ```cpp
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
  ```

- `IxIndexHandle::AdjustRoot` 删除键的辅助函数，修改根节点。
  
  ```cpp
  
  ```
  
  
