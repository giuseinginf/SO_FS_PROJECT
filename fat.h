#include "disk.h"

#include <stdio.h>

typedef struct {
    char name[32];        // file name
    uint32_t size;        // size in bytes
    uint16_t first_block; // first block index
} FileEntry;

typedef struct {
    uint16_t fat[FAT_NUM_BLOCKS];    // FAT: max 256 blocks
    FileEntry files[16];  // directory: max 16 files
} FAT;

FAT* fat_init(uint32_t num_blocks);

/*
//fat read
FAT* fat_read(Disk d);

//fat write
void fat_write(Disk d, FAT *fat);
*/