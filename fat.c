#include "fat.h"
#include "disk.h"

#define FAT_FREE -1
#define FAT_EOC -2

FAT* fat_init(uint32_t num_blocks) {
    
    FAT* fat;

    for (int i = 0; i < 256; i++) {
        fat->fat[i] = FAT_FREE;
    }

    for (int i = 0; i < 16; i++) {
        fat->files[i].name[0] = '\0';
        fat->files[i].size = 0;
        fat->files[i].first_block = FAT_EOC;
    }
    printf("FAT: fat_init completed successfully\n");
    //print fat info
    printf("FAT: Number of blocks: %u\n", num_blocks);
    return fat;
}
/*
FAT* fat_read(Disk d) {
    if (!d.base) return NULL;
    
    FAT *fat = malloc(sizeof(FAT));
    if (!fat) return NULL;
    
    memcpy(fat, d.base, sizeof(FAT));
    return fat;
    printf("FAT: fat_read completed successfully\n");
}


void fat_write(Disk d, FAT *fat) {
    if (!d.base || !fat) return;
    
    memcpy(d.base, fat, sizeof(FAT));
    printf("FAT: fat_write completed successfully\n");
}
*/