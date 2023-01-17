#include "lru_replacer.h"

LRUReplacer::LRUReplacer(size_t num_pages) { max_size_ = num_pages; }

LRUReplacer::~LRUReplacer() = default;

/**
 * @brief 使用LRU策略删除一个victim frame，这个函数能得到frame_id
 * @param[out] frame_id id of frame that was removed, nullptr if no victim was found
 * @return true if a victim frame was found, false otherwise
 */
bool LRUReplacer::Victim(frame_id_t *frame_id) {
    // C++17 std::scoped_lock
    // 它能够避免死锁发生，其构造函数能够自动进行上锁操作，析构函数会对互斥量进行解锁操作，保证线程安全。
    std::scoped_lock lock{latch_};

    if( LRUlist_.size() == 0 ) return false;

    LRUhash_.erase( LRUhash_.find( (*frame_id) = LRUlist_.back() ) );
    LRUlist_.pop_back();
    return true;
}

/**
 * @brief 固定一个frame, 表明它不应该成为victim（即在replacer中移除该frame_id）
 * @param frame_id the id of the frame to pin
 */
void LRUReplacer::Pin(frame_id_t frame_id) {
    std::scoped_lock lock{latch_};

    auto it = LRUhash_.find(frame_id);
    if( it != LRUhash_.end() ) {
        LRUlist_.erase( (*it).second );
        LRUhash_.erase( it ); 
    }
}

#include<iostream>
/**
 * 取消固定一个frame, 表明它可以成为victim（即将该frame_id添加到replacer）
 * @param frame_id the id of the frame to unpin
 */
void LRUReplacer::Unpin(frame_id_t frame_id) {
    std::scoped_lock lock{latch_};

    if( LRUhash_.find(frame_id) != LRUhash_.end() ) return ;

    LRUlist_.push_front( frame_id );
    LRUhash_[frame_id] = LRUlist_.begin();
}

/** @return replacer中能够victim的数量 */
size_t LRUReplacer::Size() {
    std::scoped_lock lock{latch_};
    return LRUlist_.size();
}
