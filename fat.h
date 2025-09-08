#include "disk.h"

#include <stdio.h>

typedef struct FileVector {
    uint32_t indexes[MAX_FILE_BLOCKS];
} FileVector;

typedef struct FAT {
    uint32_t num_blocks;
    FileVector files[MAX_FILES];
} FAT;

void fat_init(FAT *fat, uint32_t size);

//fat read
void* fat_read(Disk d);

//fat write
void fat_write(Disk d, FAT *fat);