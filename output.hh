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

class output
{
    int fd_;
    
    uint8_t* ptr_;
    uint8_t* dptr_;
    const char* fn_;
    size_t write_size_;

public:
    inline output(const char* fn):
    fd_(-1),
    ptr_(nullptr),
    dptr_(nullptr),
    fn_(fn),
    write_size_(0) {}
    
    inline ~output()
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
    
    inline int init(size_t size)
    {
        int fd = open(fn_, O_WRONLY | O_CREAT | O_TRUNC);
        if (fd == -1)
            return -1;
        
        uint8_t* m = new uint8_t[size];
        if (!m)
            return -1;

        write_size_ = size;
        
        fd_ = fd;
        ptr_ = m;
        dptr_ = m;
        
        return 0;
    }
    
    inline ssize_t write_section()
    {
        ssize_t ret = ::write(fd_, ptr_, dptr_ - ptr_);
        dptr_ = ptr_;

        return ret; 
    }

    inline bool is_filled(uint32_t add)
    {
        return (dptr_ + add) > (ptr_ + write_size_);
    }

    inline void push8(uint8_t val)
    {
        *dptr_++ = val;
    }

    inline void push16(uint16_t val)
    {
        *(uint16_t*)dptr_ = val;
        dptr_ += 2;
    }
    
    inline void push32(uint32_t val)
    {
        *(uint32_t*)dptr_ = val;
        dptr_ += 4;
    }
    
    inline void push64(uint64_t val)
    {
        *(uint64_t*)dptr_ = val;
        dptr_ += 8;
    }
};
