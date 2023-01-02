#include <fstream>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

using namespace std;

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

int main(int argc, const char* argv[])
{
    FILE* fp = fopen("../../wiki2018.trb", "rb");
    FILE* fq = fopen("../../wiki2018.tr2", "wb");
    fseek(fp, 0L, SEEK_END);
    size_t sz = ftell(fp);
    fseek(fp, 0L, SEEK_SET);
    uint32_t buf[3];
    uint32_t maxid = 0, maxsize = 0;
    size_t lines = sz / 12;
    uint32_t key_bits = 26, size_bits = 31;

    fwrite(&lines,8,1,fq);
    fwrite(&key_bits,4,1,fq);
    fwrite(&size_bits,4,1,fq);

    for (size_t idx = 0; idx < lines; ++idx) {
        fread(buf, 12, 1, fp);

        uint32_t id = buf[1];
        uint32_t size = buf[2];

        maxid = (id > maxid) ? id : maxid;
        maxsize = (size > maxsize) ? size : maxsize;

        for (int b = 0; b < 26; ++b) {
            WriteBit(maxid & (1 << b), fq);
        }
        for (int b = 0; b < 31; ++b) {
            WriteBit(maxsize & (1 << b), fq);
        }
    }

    Flush_Bits(fq);
    printf("%u:%u\n", maxid,maxsize);

    fclose(fp);
    fclose(fq);

    return 0;
}
