#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include "tsl/htrie_map.h"
#include "mum.h"
#include <string>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define WRITE_EXT() \
    memcpy(fn_out, argv[1], sz); \
    memcpy(fn_out + sz, ".trb", 4); \
    fn_out[sz + 4] = '\0';

#define EXIT_MSG(msg, len) \
    write(2, msg, len); \
    exit(EXIT_FAILURE);

#define MAX_RW_SIZE (1UL << 30)

struct str_hash {
    std::size_t operator()(const char* key, std::size_t key_size) const {
        return mum_hash(key, key_size, 0X7e14eadb1f65311d);
    }
};

uint32_t itoa(char** data)
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

int main(int argc, const char* argv[])
{
    if (argc != 2) {
        EXIT_MSG("usage: tcbw INPUT\n", 18)
    }

    tsl::htrie_map<char, uint32_t, str_hash> keys;
    size_t sz = strlen(argv[1]);
    char fn_out[sz + 5];

    WRITE_EXT()

    int fd_twtr = open(argv[1], O_RDONLY);
    int fd_bin  = open(fn_out, O_WRONLY | O_CREAT | O_TRUNC);

    if (fd_twtr == -1) {
        close(fd_bin);
        EXIT_MSG("in file not openable\n", 21)
    }

    if (fd_bin == -1) {
        close(fd_twtr);
        EXIT_MSG("out file not openable\n", 22)
    }


    struct stat buf;
    fstat(fd_twtr, &buf);

    char* input = new char[MAX_RW_SIZE];
    uint32_t* output = new uint32_t[MAX_RW_SIZE>>2];
    uint32_t used;
    char* stop = input + MAX_RW_SIZE;
    //vector<uint32_t> output;


    do {
        uint32_t read_size = (buf.st_size < MAX_RW_SIZE) ? buf.st_size : MAX_RW_SIZE;
        if (read(fd_twtr, input, read_size) == -1) {
            truncate(fn_out, 0);
            EXIT_MSG("file not readable\n", 18)
        }

        // write data
        char* data = input;
        do {
            uint32_t ts = itoa(&data);
            char* end = (char*)memchr(data, ',', 32);
            uint32_t id = 0;


            auto it = keys.find_ks(data, end - data);
            if (it != keys.end()) {
                id = it.value();
            } else {
                id = uint32_t(keys.size());
                keys.insert_ks(data, end - data, keys.size());
            }


            char* sizes = end + 1;
            uint32_t key_size   = itoa(&sizes);
            uint32_t value_size = itoa(&sizes);

/*
            write(2,data, end - data);
            printf("\n%u:%u\n", ts, id);
            printf("%u:%u\n", key_size, value_size);
*/

            while (*data != '\n') ++data;
            ++data;

            output[used + 0] = ts;
            output[used + 1] = id;
            output[used + 2] = (key_size + value_size);
            used += 3;
        } while (stop - data > 32);

        if (used >= MAX_RW_SIZE) {
            write(fd_bin,output,used); 
            //output.clear();
            used = 0;
        }

        buf.st_size -= read_size;
    } while (buf.st_size > 0);


    close(fd_twtr);
    close(fd_bin);

    exit(EXIT_SUCCESS);
}
