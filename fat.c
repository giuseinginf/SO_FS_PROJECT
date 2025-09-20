#include "fat.h"

int write_metainfo(void *disk_mem, const DiskInfo *info, size_t block_size, size_t disk_size_bytes) {
    char buffer[block_size];
    memset(buffer, 0, block_size);
    memcpy(buffer, info, sizeof(DiskInfo));
    return write_block(disk_mem, 0, buffer, block_size, disk_size_bytes);
}

int read_metainfo(void *disk_mem, DiskInfo *info, size_t block_size, size_t disk_size_bytes) {
    char buffer[block_size];
    int res = read_block(disk_mem, 0, buffer, block_size, disk_size_bytes);
    if (res != 0) return res;
    memcpy(info, buffer, sizeof(DiskInfo));
    return 0;
}

// --- FAT ---

int write_fat(void *disk_mem, const uint32_t *fat, uint32_t num_fat_entries, uint32_t start_block, size_t block_size, size_t disk_size_bytes) {
    
    uint32_t fat_bytes = num_fat_entries * sizeof(uint32_t);
    uint32_t fat_blocks = (fat_bytes + block_size - 1) / block_size;

    for (uint32_t i = 0; i < fat_blocks; i++) {
        char buffer[block_size];
        memset(buffer, 0, block_size);
        size_t offset = i * block_size;
        size_t bytes_to_copy = block_size;
        if (offset + bytes_to_copy > fat_bytes)
            bytes_to_copy = fat_bytes - offset;
        memcpy(buffer, ((char*)fat) + offset, bytes_to_copy);
        int res = write_block(disk_mem, start_block + i, buffer, block_size, disk_size_bytes);
        if (res != 0) return res;
    }
    return 0;
}

int read_fat(void *disk_mem, uint32_t *fat, uint32_t num_fat_entries, uint32_t start_block, size_t block_size, size_t disk_size_bytes) {
    uint32_t fat_bytes = num_fat_entries * sizeof(uint32_t);
    uint32_t fat_blocks = (fat_bytes + block_size - 1) / block_size;

    for (uint32_t i = 0; i < fat_blocks; i++) {
        char buffer[block_size];
        int res = read_block(disk_mem, start_block + i, buffer, block_size, disk_size_bytes);
        if (res != 0) return res;
        size_t offset = i * block_size;
        size_t bytes_to_copy = block_size;
        if (offset + bytes_to_copy > fat_bytes)
            bytes_to_copy = fat_bytes - offset;
        memcpy(((char*)fat) + offset, buffer, bytes_to_copy);
    }
    return 0;
}