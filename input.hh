#pragma once

#include <sys/mman.h>
#include <cstdlib>
#include <cstring>
#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string_view>

class input
{
    int fd_;
    
    uint8_t* ptr_;
    const char* fn_;
    
    size_t block_size_;
    size_t total_size_;
    size_t finished_size_;

public:
    inline input(const char* fn):
    fd_(-1),
    ptr_(nullptr),
    fn_(fn),
    block_size_(0),
    total_size_(0),
    finished_size_(0) {}
    
    inline ~input()
    {
        if (fd_ != -1)
            close(fd_);
        
        if (ptr_)
            delete[] ptr_;
    }
    
    inline uint8_t* data()
    {
        return ptr_;
    }
    
    int init(size_t size)
    {
        int fd = open(fn_, O_RDONLY);
        if (fd == -1)
            return -1;
        
        struct stat st;
        if (fstat(fd, &st) == -1)
            return -1;
        
        total_size_ = st.st_size;

        
        uint8_t* m = new uint8_t[size];
        if (!m)
            return -1;
        
        fd_ = fd;
        ptr_ = m;
        block_size_ = (size <= total_size_) ? size : total_size_;
        finished_size_ = 0;
        
        return 0;
    }
    
    size_t read_section()
    {
        size_t read_size = ((finished_size_ + block_size_) < total_size_) ? block_size_ : (total_size_ - finished_size_);
        ssize_t rr = ::read(fd_, ptr_, read_size);
        if (rr == -1)
            return -1;
        
        finished_size_ += read_size;
        
        return read_size;
    }
    
    inline bool has_next()
    {
        return total_size_ > finished_size_;
    }
};
