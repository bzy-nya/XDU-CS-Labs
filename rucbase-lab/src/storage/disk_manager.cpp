#include "storage/disk_manager.h"

#include <assert.h>    // for assert
#include <string.h>    // for memset
#include <sys/stat.h>  // for stat
#include <unistd.h>    // for lseek

#include "defs.h"

DiskManager::DiskManager() { memset((char *)fd2pageno_, 0, MAX_FD * (sizeof(std::atomic<page_id_t>) / sizeof(char))); }

/**
 * @brief Write the contents of the specified page into disk file
 *
 */
void DiskManager::write_page(int fd, page_id_t page_no, const char *offset, int num_bytes) {
    if( fd2path_.find(fd) == fd2path_.end() ) {
        throw FileNotOpenError(fd);
    }
    lseek(fd, page_no * PAGE_SIZE, SEEK_SET);
    if( write(fd, (void *)offset, num_bytes) != num_bytes ) {
        throw UnixError();
    }
}

/**
 * @brief Read the contents of the specified page into the given memory area
 */
void DiskManager::read_page(int fd, page_id_t page_no, char *offset, int num_bytes) {
    if( fd2path_.find(fd) == fd2path_.end() ) {
        throw FileNotOpenError(fd);
    }
    lseek(fd, page_no * PAGE_SIZE, SEEK_SET);
    if( read(fd, (void *)offset, num_bytes) < 0 ) {
        throw UnixError();
    }
}

/**
 * @brief Allocate new page (operations like create index/table)
 * For now just keep an increasing counter
 */
page_id_t DiskManager::AllocatePage(int fd) {
	return fd2pageno_[fd] ++; //
}

/**
 * @brief Deallocate page (operations like drop index/table)
 * Need bitmap in header page for tracking pages
 * This does not actually need to do anything for now.
 */
void DiskManager::DeallocatePage(__attribute__((unused)) page_id_t page_id) {}

bool DiskManager::is_dir(const std::string &path) {
    struct stat st;
    return stat(path.c_str(), &st) == 0 && S_ISDIR(st.st_mode);
}

void DiskManager::create_dir(const std::string &path) {
    // Create a subdirectory
    std::string cmd = "mkdir " + path;
    if (system(cmd.c_str()) < 0) {  // 创建一个名为path的目录
        throw UnixError();
    }
}

void DiskManager::destroy_dir(const std::string &path) {
    std::string cmd = "rm -r " + path;
    if (system(cmd.c_str()) < 0) {
        throw UnixError();
    }
}

/**
 * @brief 用于判断指定路径文件是否存在 
 */
bool DiskManager::is_file(const std::string &path) {
    struct stat st;
    return stat(path.c_str(), &st) == 0 && (S_IFREG ==(st.st_mode & S_IFMT));
}

/**
 * @brief 用于创建指定路径文件
 */
void DiskManager::create_file(const std::string &path) {
    if(!is_file(path)) {
        auto fd = open( path.c_str(), O_CREAT | O_RDWR, 0x1ff );
        close(fd);
    } else {
        throw FileExistsError(path);
    }
}

/**
 * @brief 用于删除指定路径文件 
 */
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

/**
 * @brief 用于打开指定路径文件
 */
int DiskManager::open_file(const std::string &path) {
    if( !is_file(path) ) {
        throw FileNotFoundError(path);
    }
    auto it = path2fd_.find(path);
    if( it == path2fd_.end() ) {
        auto fd = open(path.c_str(), O_RDWR);
        if( fd < 0 ) {
            throw UnixError();
        }
        path2fd_[path] = fd;
        fd2path_[fd] = path;
        return fd;
    } else {
        throw FileNotClosedError(path);
    }
}

/**
 * @brief 用于关闭指定路径文件
 */
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

int DiskManager::GetFileSize(const std::string &file_name) {
    struct stat stat_buf;
    int rc = stat(file_name.c_str(), &stat_buf);
    return rc == 0 ? stat_buf.st_size : -1;
}

std::string DiskManager::GetFileName(int fd) {
    if (!fd2path_.count(fd)) {
        throw FileNotOpenError(fd);
    }
    return fd2path_[fd];
}

int DiskManager::GetFileFd(const std::string &file_name) {
    if (!path2fd_.count(file_name)) {
        return open_file(file_name);
    }
    return path2fd_[file_name];
}

bool DiskManager::ReadLog(char *log_data, int size, int offset, int prev_log_end) {
    // read log file from the previous end
    if (log_fd_ == -1) {
        log_fd_ = open_file(LOG_FILE_NAME);
    }
    offset += prev_log_end;
    int file_size = GetFileSize(LOG_FILE_NAME);
    if (offset >= file_size) {
        return false;
    }

    size = std::min(size, file_size - offset);
    lseek(log_fd_, offset, SEEK_SET);
    ssize_t bytes_read = read(log_fd_, log_data, size);
    if (bytes_read != size) {
        throw UnixError();
    }
    return true;
}

void DiskManager::WriteLog(char *log_data, int size) {
    if (log_fd_ == -1) {
        log_fd_ = open_file(LOG_FILE_NAME);
    }

    // write from the file_end
    lseek(log_fd_, 0, SEEK_END);
    ssize_t bytes_write = write(log_fd_, log_data, size);
    if (bytes_write != size) {
        throw UnixError();
    }
}
