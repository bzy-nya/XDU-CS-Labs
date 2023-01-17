#include "rm_file_handle.h"

/**
 * @brief 由Rid得到指向RmRecord的指针
 *
 * @param rid 指定记录所在的位置
 * @return std::unique_ptr<RmRecord>
 */
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

/**
 * @brief 在该记录文件（RmFileHandle）中插入一条记录
 *
 * @param buf 要插入的数据的地址
 * @return Rid 插入记录的位置
 */
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

/**
 * @brief 在该记录文件（RmFileHandle）中删除一条指定位置的记录
 *
 * @param rid 要删除的记录所在的指定位置
 */
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

/**
 * @brief 更新指定位置的记录
 *
 * @param rid 指定位置的记录
 * @param buf 新记录的数据的地址
 */
void RmFileHandle::update_record(const Rid &rid, char *buf, Context *context) {

    //std::cout << "update recoder {" << rid.page_no << " " << rid.slot_no << "}\n";
 
    auto pageHandler = fetch_page_handle(rid.page_no);

    if( !Bitmap::is_set(pageHandler.bitmap, rid.slot_no) ) {
        throw RecordNotFoundError(rid.page_no, rid.slot_no);
    }
    memcpy( pageHandler.get_slot(rid.slot_no), buf, file_hdr_.record_size );
}

/** -- 以下为辅助函数 -- */
/**
 * @brief 获取指定页面编号的page handle
 *
 * @param page_no 要获取的页面编号
 * @return RmPageHandle 返回给上层的page_handle
 * @note pin the page, remember to unpin it outside!
 */
RmPageHandle RmFileHandle::fetch_page_handle(int page_no) const {
    if( page_no >= file_hdr_.num_pages ) {
        throw PageNotExistError(disk_manager_ -> GetFileName(fd_), page_no);
    }
    return RmPageHandle(&file_hdr_, buffer_pool_manager_ -> FetchPage( {fd_, page_no} ));
}

/**
 * @brief 创建一个新的page handle
 *
 * @return RmPageHandle
 */
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

/**
 * @brief 创建或获取一个空闲的page handle
 *
 * @return RmPageHandle 返回生成的空闲page handle
 * @note pin the page, remember to unpin it outside!
 */
RmPageHandle RmFileHandle::create_page_handle() {
    if(file_hdr_.first_free_page_no == RM_NO_PAGE) {
        return create_new_page_handle();
    }
    return fetch_page_handle(file_hdr_.first_free_page_no);
}

/**
 * @brief 当page handle中的page从已满变成未满的时候调用
 *
 * @param page_handle
 * @note only used in delete_record()
 */
void RmFileHandle::release_page_handle(RmPageHandle &page_handle) {
    page_handle.page_hdr -> next_free_page_no = file_hdr_.first_free_page_no;
    file_hdr_.first_free_page_no = page_handle.page -> GetPageId().page_no;
}

// used for recovery (lab4)
void RmFileHandle::insert_record(const Rid &rid, char *buf) {
    if (rid.page_no < file_hdr_.num_pages) {
        create_new_page_handle();
    }
    RmPageHandle pageHandle = fetch_page_handle(rid.page_no);
    Bitmap::set(pageHandle.bitmap, rid.slot_no);
    pageHandle.page_hdr->num_records++;
    if (pageHandle.page_hdr->num_records == file_hdr_.num_records_per_page) {
        file_hdr_.first_free_page_no = pageHandle.page_hdr->next_free_page_no;
    }

    char *slot = pageHandle.get_slot(rid.slot_no);
    memcpy(slot, buf, file_hdr_.record_size);

    buffer_pool_manager_->UnpinPage(pageHandle.page->GetPageId(), true);
}