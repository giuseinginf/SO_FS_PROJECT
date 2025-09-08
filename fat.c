#include "fat.h"
#include "disk.h"

#define BLOCK_FREE -1
#define EOF -2

void fat_init(FAT *fat, uint32_t size){
    fat->num_blocks = FAT_SIZE / BLOCK_SIZE; // 1 MB / 4 KB = 256
    fat->block_size = BLOCK_SIZE;
    for (int i = 0; i < MAX_FILES; i++) {
        for (int j = 0; j < MAX_FILE_BLOCKS; j++) {
            fat->files[i].indexes[j] = BLOCK_FREE;
        }
    }
    printf("FAT: function fat_init completed successfully\n");
}

void* fat_read(Disk *d, FAT *fat) {
    //compute index
    void* buffer = malloc(FAT_SIZE);
    if(!buffer) {
        perror("Failed to allocate memory");
        exit(EXIT_FAILURE);
    }
    
    disk_read(d, FAT_START_BLOCK, buffer);
    printf("FAT: function fat_read completed successfully\n");
    return buffer;
}

void fat_write(Disk *d, FAT *fat) {
    disk_write(d, FAT_START_BLOCK, fat, FAT_SIZE);
    printf("FAT: function fat_write completed successfully\n");
}