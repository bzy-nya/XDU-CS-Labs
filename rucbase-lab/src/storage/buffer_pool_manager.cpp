#include "buffer_pool_manager.h"

/**
 * @brief 从free_list或replacer中得到可淘汰帧页的 *frame_id
 * @param frame_id 帧页id指针,返回成功找到的可替换帧id
 * @return true: 可替换帧查找成功 , false: 可替换帧查找失败
 */
bool BufferPoolManager::FindVictimPage(frame_id_t *frame_id) {
    if( free_list_.size() != 0 ) {
        (*frame_id) = free_list_.front();
        free_list_.pop_front();
        //std::cout << "alloc new frame " << *frame_id << "]]]]]\n";
        return true;
    }
    return replacer_ -> Victim( frame_id );
}

/**
 * @brief 更新页面数据, 为脏页则需写入磁盘，更新page元数据(data, is_dirty, page_id)和page table
 *
 * @param new_page_id 写回页新page_id
 * @param new_frame_id 写回页新帧frame_id
 */
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

/**
 * Fetch the requested page from the buffer pool.
 * 如果页表中存在page_id（说明该page在缓冲池中），并且pin_count++。
 * 如果页表不存在page_id（说明该page在磁盘中），则找缓冲池victim page，将其替换为磁盘中读取的page，pin_count置1。
 * @param page_id id of page to be fetched
 * @return the requested page
 */
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

/**
 * Unpin the target page from the buffer pool. 取消固定pin_count>0的在缓冲池中的page
 * @param page_id id of page to be unpinned
 * @param is_dirty true if the page should be marked as dirty, false otherwise
 * @return false if the page pin count is <= 0 before this call, true otherwise
 */
bool BufferPoolManager::UnpinPage(PageId page_id, bool is_dirty) {
    std::scoped_lock lock{latch_};

    if( page_table_.find(page_id) == page_table_.end() ) return false;
    frame_id_t frame_id = page_table_[page_id];

    Page *page = &pages_[frame_id];

    if( page -> pin_count_ == 0 ) return false;
    if( -- page -> pin_count_ == 0 ) replacer_ -> Unpin(frame_id);
    
    page -> is_dirty_ |= is_dirty;

    return true;
}

/**
 * Flushes the target page to disk. 将page写入磁盘；不考虑pin_count
 * @param page_id id of page to be flushed, cannot be INVALID_PAGE_ID
 * @return false if the page could not be found in the page table, true otherwise
 */
bool BufferPoolManager::FlushPage(PageId page_id) {
    std::scoped_lock lock{latch_};

    if( page_table_.find(page_id) == page_table_.end() ) return false;

    Page *page = &pages_[page_table_[page_id]];
    disk_manager_ -> write_page(page -> GetPageId().fd, page -> GetPageId().page_no, page -> GetData(), PAGE_SIZE);

    page -> is_dirty_ = false;

    return true;
}

/**
 * Creates a new page in the buffer pool. 相当于从磁盘中移动一个新建的空page到缓冲池某个位置
 * @param[out] page_id id of created page
 * @return nullptr if no new pages could be created, otherwise pointer to new page
 */
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

/**
 * @brief Deletes a page from the buffer pool.
 * @param page_id id of page to be deleted
 * @return false if the page exists but could not be deleted, true if the page didn't exist or deletion succeeded
 */
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

/**
 * @brief Flushes all the pages in the buffer pool to disk.
 *
 * @param fd 指定的diskfile open句柄
 */
void BufferPoolManager::FlushAllPages(int fd) {
    // example for disk write
    std::scoped_lock lock{latch_};
    for (size_t i = 0; i < pool_size_; i++) {
        Page *page = &pages_[i];
        if (page->GetPageId().fd == fd && page->GetPageId().page_no != INVALID_PAGE_ID) {
            disk_manager_->write_page(page->GetPageId().fd, page->GetPageId().page_no, page->GetData(), PAGE_SIZE);
            page->is_dirty_ = false;
        }
    }
}