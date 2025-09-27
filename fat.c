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

void init_fat(uint32_t* fat, uint32_t num_entries) {
    for (uint32_t i = 0; i < num_entries; i++) {
        //we put EOF at the end of the FAT
        if (i == num_entries - 1) {
            fat[i] = FAT_EOF;
            continue;
        }
        fat[i] = i + 1; // Initialize FAT entries
    }
}

void printFat(const uint32_t* fat, uint32_t num_entries) {
    for (uint32_t i = 0; i < num_entries; i++) {
        printf("FAT[%u] = %u\n", i, fat[i]);
    }
}

int update_fat_and_metainfo(void *disk_mem, uint32_t *fat, uint32_t num_fat_entries, uint32_t fat_start_block, DiskInfo *info, size_t block_size, size_t disk_size_bytes) {
    int res = write_fat(disk_mem, fat, num_fat_entries, fat_start_block, block_size, disk_size_bytes);
    if (res != 0) return res;
    return write_metainfo(disk_mem, info, block_size, disk_size_bytes);
}

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

uint32_t get_free_block(const DiskInfo* info) {
    return info->free_list_head;
}

int allocateBlock(uint32_t* fat, DiskInfo* info) {
    uint32_t freeListHead = info->free_list_head;
    uint32_t free_blocks = info->free_blocks;
    if (free_blocks == 0) {
        return FAT_EOF; // No free blocks available
    }
    uint32_t allocatedBlock = freeListHead;
    freeListHead = fat[allocatedBlock];  // new free list head
    fat[allocatedBlock] = FAT_EOC;        // Mark block as end of chain
    info->free_list_head = freeListHead;
    info->free_blocks--;
    return allocatedBlock;
}

int appendBlockToChain(uint32_t* fat, DiskInfo* info, uint32_t chainHead) {
    // 1. Allocate a new block from the free list
    uint32_t freeListHead = info->free_list_head;
    if (freeListHead == FAT_EOF) {
        // No free blocks available
        return FAT_EOF;
    }
    uint32_t newBlock = freeListHead;
    freeListHead = fat[newBlock];
    fat[newBlock] = FAT_EOC;

    // 2. Find the last block in chain
    uint32_t cur = chainHead;
    while (fat[cur] != FAT_EOC) {
        cur = fat[cur];
    }
    // 3. link the last block to the new block
    fat[cur] = newBlock;
    info->free_list_head = freeListHead;
    info->free_blocks--;
    return newBlock;
}

int deallocateChain(uint32_t* fat, DiskInfo* info, uint32_t start) {
    uint32_t block = start;
    uint32_t freeListHead = info->free_list_head;
    while (block != FAT_EOC) {
        uint32_t next = fat[block];
        fat[block] = freeListHead;   // Concatenate to free list
        freeListHead = block;        // New head of free list
        info->free_blocks++;
        if (next == FAT_EOC)
            break;
        block = next;
    }
    info->free_list_head = freeListHead;
    return 0;
}