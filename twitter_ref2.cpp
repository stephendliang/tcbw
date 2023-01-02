#include <fstream>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include "tsl/htrie_map.h"
#include "mum.h"
#include "input.hh"
#include "output.hh"
#include <string>
#include <iostream>

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

int current_bit = 0;
unsigned char bit_buffer = 0;

void WriteBit (int bit, FILE* fd)
{
  if (bit)
    bit_buffer |= (1<<current_bit);

  current_bit++;
  if (current_bit == 8)
  {
    fwrite (&bit_buffer, 1, 1, fd);
    current_bit = 0;
    bit_buffer = 0;
  }
}

void Flush_Bits (FILE* fd)
{
  while (current_bit) 
    WriteBit (0, fd);
}

tsl::htrie_map<char, uint32_t, str_hash> keys_;

#define PARSE_LINE() \
    char *data = line; \
    uint32_t ts = itoa(&data), id = 0; \
    char* endts = (char*)memchr(data, ',', 256); \
    auto it = keys_.find_ks(data, endts - data); \
    if (it != keys_.end()) { \
        id = it.value(); \
    } else { \
        id = keys_.size(); \
        keys_.insert_ks(data, endts - data, id); \
    } \
    char* sizes         = endts + 1; \
    uint32_t key_size   = itoa(&sizes); \
    uint32_t value_size = itoa(&sizes); \
    uint32_t sum        = key_size + value_size;


#define REPARSE_LINE() \
    char *data = line; \
    uint32_t ts = itoa(&data), id = 0; \
    char* endts = (char*)memchr(data, ',', 256); \
    id = keys_.find_ks(data, endts - data).value(); \
    char* sizes         = endts + 1; \
    uint32_t key_size   = itoa(&sizes); \
    uint32_t value_size = itoa(&sizes); \
    uint32_t sum        = key_size + value_size;



void read_c(FILE * ifile,FILE* fo0) //, , , FILE* out1file
{
    size_t linesz = 4096+1;
    char * line = new char[linesz];
    uint32_t lines_size = 0;
    uint32_t max_size = 0;
    uint32_t key_bits = 0, size_bits = 0;
    //while(getline(&line, &linesz, ifile) > 0) {
    // PARSE_LINE()
    // max_size            = (sum > max_size) ? sum : max_size;
    //}

    fwrite(&lines_size,4,1,out0file);
    fwrite(&key_bits,4,1,out0file);
    fwrite(&size_bits,4,1,out0file);

    while(getline(&line, &linesz, ifile) > 0) {

        for (int bits = 21; bits >= 0; --bits) {
            WriteBit(id & (1 << bits), fo0);
        }
        for (int bits = 24; bits >= 0; --bits) {
            WriteBit(sum & (1 << bits), fo0);
        }
/*
        fwrite(&id,4,1,out0file);
        fwrite(&sum,4,1,out0file);

        fwrite(&ts,4,1,out1file);
        fwrite(&id,4,1,out1file);
        fwrite(&sum,4,1,out1file);
*/
        lines_size++;
    }

    delete[] line;
    printf("Lines count:%d\n", max_size);
    printf("Lines count:%d\n", keys_.size());
    Flush_Bits(fo0);
    /*
        while (fscanf(fp, "%u,%s,%u,%u,%u,%s,%u", &timestamp, key, &ksize, &vsize, &client, operation, &ttl) == 7) {
            printf("%u,%s,%u,%u,%u,%s,%u", timestamp, key, ksize, vsize, client, operation, ttl);
        }
    */
}


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

    //std::ifstream in_file(argv[1]);
    //std::ofstream out_file8(fn_out8);
    //std::ofstream out_file12(fn_out12);
    //const char* timestamp,* key,* ksize,* vsize,* client,* operation,* ttl;
    //uint32_t timestamp, ksize, vsize, client, ttl;
    //char key[24], operation[8];



//0,ON7pp7JN4NHJOpoJJ4J,19,74,4,get,0
    FILE *fp = fopen(argv[1], "r");
    FILE *fo0 = fopen(fn_out8, "wb");
    //FILE *fp = fopen(fn_out12, "r");

    if (fp) {
        read_c(fp, fo0);
        fclose(fp);
    } else {
        perror("fopen");
        throw;
    }


/*
    while (in_file >> timestamp >> key >> ksize >> vsize >> client >> operation >> ttl)
    {
        std::cout << timestamp << key << ksize << vsize << client << operation << ttl << "\n\n";
        throw;

        uint32_t timestamp_u32 = itoa(timestamp.data());

        auto it = keys_.find(key);
        if (it != keys_.end()) {
            key_u32 = it.value();
        } else {
            key_u32 = keys_.size();
            keys_.insert(key, key_u32); 
        }

        uint32_t ksize_u32 = itoa(&sizes);
        uint32_t vsize_u32 = itoa(&sizes);


        out12.push32(ts);
        out12.push32(id);
        out12.push32(key_size + value_size);
    }
*/
    return 0;
}
