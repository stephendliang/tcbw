#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include "tsl/htrie_map.h"
#include "mum.h"
#include "input.hh"
#include "output.hh"
#include <string>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


#define WRITE_EXT(buffer, ext, len) \
    memcpy(buffer, argv[1], sz); \
    memcpy(buffer + sz, ext, len); \
    buffer[sz + len] = '\0';

#define EXIT_MSG(msg, len) \
    write(2, msg, len); \
    exit(EXIT_FAILURE);

#define MAX_RW_SIZE     (1UL << 30)
#define MAX_RW_U32_SIZE (MAX_RW_SIZE >> 2)
#define KEYS_OFFSET     87381

struct str_hash
{
    inline size_t operator()(const char* key, std::size_t key_size) const
    {
        return mum_hash(key, key_size, 0X7e14eadb1f65311d);
    }
};

static uint32_t itoa(char** data)
{
    char* p = *data;
    uint32_t res = 0;

    while (*p != ',') {
        res *= 10;
        res += (*p ^ '0');
        ++p;
    }

    ++p;
    *data = p;
    return res;
}

static uint32_t itoa_break(char** data, const char* stop)
{
    char* p = *data;
    uint32_t res = 0;

    while (((*p ^ '0') < 10) && p < stop) {
        res *= 10;
        res += (*p ^ '0');
        ++p;
    }

    if (p >= stop) {
        uint32_t len = uint32_t(p - *data);
        return len | 0x80000000;
    }

    ++p;
    *data = p;
    return res;
}



#define NOW() std::chrono::steady_clock::now();
#define DURATION(A,B) std::chrono::duration_cast<std::chrono::nanoseconds>(B-A).count();

class twitter_conv
{
    tsl::htrie_map<char, uint32_t, str_hash> keys_;

    input inp;
    output out8;
    output out12;

    char savebuf[32];
    char* in_data;
    uint32_t* out_data;
    char* stop;
    int stopmark = 0;

    char* parse_line_break(char* data)
    {
        puts(data);

        uint32_t ts = itoa_break(&data, stop);
        char* endts = (char*)memchr(data, ',', 32);
        uint32_t id = 0;

        auto it = keys_.find_ks(data, endts - data);
        if (it != keys_.end()) {
            id = it.value();
        } else {
            id = keys_.size();
            keys_.insert_ks(data, endts - data, id); 
        }

        char* sizes = endts + 1;
        uint32_t key_size   = itoa_break(&sizes, stop);
        if (key_size & 0x80000000) {
            puts(data);
            //key_size & 0x7FFFFFFF
            memcpy(savebuf, sizes, key_size & 0x7FFFFFFF);
            stopmark = 1;
            return nullptr;
        }

        uint32_t value_size = itoa_break(&sizes, stop);

        while (*data != '\n' && data <= stop) ++data;
        if (data >= stop) {}

        ++data;

        out12.push32(ts);
        out12.push32(id);
        out12.push32(key_size + value_size);

        return data;
    }

    inline char* parse_line(char* data)
    {
        uint32_t ts = itoa(&data);
        char* endts = (char*)memchr(data, ',', 32);
        uint32_t id = 0;

        auto it = keys_.find_ks(data, endts - data);
        if (it != keys_.end()) {
            id = it.value();
        } else {
            id = keys_.size();
            keys_.insert_ks(data, endts - data, id); 
        }

        char* sizes = endts + 1;
        uint32_t key_size   = itoa(&sizes);
        uint32_t value_size = itoa(&sizes);

        while (*data != '\n') ++data;

        ++data;

        out12.push32(ts);
        out12.push32(id);
        out12.push32(key_size + value_size);

        //out8.push32(id);
        //out8.push32(key_size + value_size);

        return data;
    }

public:
    twitter_conv(const char* fn_in, const char* fn_out8, const char* fn_out12):
    inp(fn_in),
    out8(fn_out8),
    out12(fn_out12) {}

    void run()
    {
        if (inp.init(MAX_RW_SIZE) == -1) {
            EXIT_MSG("in file not openable\n", 21)
        }

        if (out12.init(MAX_RW_SIZE) == -1) {
            EXIT_MSG("out file not openable\n", 22)
        }

        do {
            if (inp.read_section() == -1) {
                EXIT_MSG("in file not readable\n", 21)
            }

            in_data  = (char*)inp.data();
            out_data = (uint32_t*)out12.data();
            stop   = in_data + MAX_RW_SIZE;

            char* data = in_data;
            


            uint32_t rows = 0;
            auto begin = NOW() \
            do {
                data = (stop - data > 100) ? parse_line(data) : parse_line_break(data);
                ++rows;
            } while (data != nullptr && stop > data && !out12.is_filled(12));
            auto end = NOW() \
            auto time = DURATION(begin, end)
            printf("%lld.%lld s\n", time/1000000000, time%1000000000);
            printf("%lld ns/row\n", time/(rows));
            puts(savebuf);

            throw;


            if (out12.is_filled(12))
                out12.write_section();
            
        } while (inp.has_next());

        out12.write_section();

    }
};

int main(int argc, const char* argv[])
{
    if (argc != 2) {
        EXIT_MSG("usage: twconv INPUT\n", 20)
    }

    size_t sz = strlen(argv[1]);
    char fn_out8[sz + 5];
    char fn_out12[sz + 5];

    WRITE_EXT(fn_out8, ".tr8", 4)
    WRITE_EXT(fn_out12, ".trc", 4)

    twitter_conv tc(argv[1], fn_out8, fn_out12);
    tc.run();

    exit(EXIT_SUCCESS);
}
