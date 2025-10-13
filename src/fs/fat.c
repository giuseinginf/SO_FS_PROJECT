#include "fat.h"

//write metainfo to disk
int write_metainfo(char* disk_mem, const DiskInfo *info, size_t block_size, size_t disk_size_bytes) {
    char buffer[block_size];
    memset(buffer, 0, block_size);
    memcpy(buffer, info, sizeof(DiskInfo));
    uint32_t index = 0; //metainfo is always at block 0
    return write_block(disk_mem, index, buffer, block_size, disk_size_bytes);
}

//read metainfo from disk
int read_metainfo(char* disk_mem, DiskInfo *info, size_t block_size, size_t disk_size_bytes) {
    char buffer[block_size];
    uint32_t index = 0; //metainfo is always at block 0
    int res = read_block(disk_mem, index, buffer, block_size, disk_size_bytes);
    if (res != 0) return res;
    memcpy(info, buffer, sizeof(DiskInfo));
    return 0;
}

//initialize FAT struct
void init_fat(uint32_t* fat, uint32_t num_entries) {
    for (uint32_t i = 0; i < num_entries; i++) {
        //mark the end of the FAT with EOF
        if (i == num_entries - 1) {
            fat[i] = FAT_EOF;
            continue;
        }
        fat[i] = i + 1;
    }
}

//print FAT entries
void print_fat(const uint32_t* fat, uint32_t num_entries) {
    for (uint32_t i = 0; i < num_entries; i++) {
        //if EOF, print "EOF" instead of the number
        if (fat[i] == FAT_EOF) {
            printf("FAT[%u] = EOF\n", i);
            continue;
        }
        //if EOC, print "EOC" instead of the number
        if (fat[i] == FAT_EOC) {
            printf("FAT[%u] = EOC\n", i);
            continue;
        }
        else printf("FAT[%u] = %u\n", i, fat[i]);
    }
}

//load metainfo and FAT from disk into RAM
void read_info_and_fat(char* disk_mem, DiskInfo* info, uint32_t* fat, size_t disk_size) {
    //read metainfo
    int res = read_metainfo(disk_mem, info, BLOCK_SIZE, disk_size);
    if (res != 0) handle_error("Failed to read metainfo");
    //read fat
    uint32_t num_fat_entries = disk_size / BLOCK_SIZE;
    uint32_t fat_index = 1; // FAT starts at block 1
    res = read_fat(disk_mem, fat, num_fat_entries, fat_index, BLOCK_SIZE, disk_size);
    if (res != 0) handle_error("Failed to read FAT");
}

//update FAT and metainfo on disk
int write_info_and_fat(char* disk_mem, uint32_t *fat, uint32_t num_fat_entries, uint32_t fat_start_block, DiskInfo *info, size_t block_size, size_t disk_size_bytes) {
    //this function calls both write_fat and write_metainfo
    int res = write_fat(disk_mem, fat, num_fat_entries, fat_start_block, block_size, disk_size_bytes);
    if (res != 0) return res;
    return write_metainfo(disk_mem, info, block_size, disk_size_bytes);
}

//write FAT to disk
int write_fat(char* disk_mem, const uint32_t *fat, uint32_t num_fat_entries, uint32_t start_block, size_t block_size, size_t disk_size_bytes) {
    //compute how many blocks are reserved for FAT
    uint32_t fat_bytes = num_fat_entries * sizeof(uint32_t);
    uint32_t fat_blocks = (fat_bytes + block_size - 1) / block_size;
    //write each block of FAT to disk
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

//read FAT from disk
int read_fat(char* disk_mem, uint32_t *fat, uint32_t num_fat_entries, uint32_t start_block, size_t block_size, size_t disk_size_bytes) {
    //compute how many blocks are reserved for FAT
    uint32_t fat_bytes = num_fat_entries * sizeof(uint32_t);
    uint32_t fat_blocks = (fat_bytes + block_size - 1) / block_size;
    //read each block of FAT from disk
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

//get the index of the first free block from metainfo
uint32_t get_list_head(const DiskInfo* info) {
    return info->free_list_head;
}

//allocate a block from the free list and update metainfo and FAT
uint32_t allocate_block(uint32_t* fat, DiskInfo* info) {
    //this function updates both FAT and metainfo
    uint32_t freeListHead = get_list_head(info);
    uint32_t free_blocks = info->free_blocks;
    if (free_blocks == 0) return FAT_EOF;     //no free blocks available
    uint32_t allocatedBlock = freeListHead;
    freeListHead = fat[allocatedBlock];       //new free list head
    fat[allocatedBlock] = FAT_EOC;            //mark block as end of chain
    info->free_list_head = freeListHead;
    info->free_blocks--;
    return allocatedBlock;
}

//append a new block to the end of a file's block chain
uint32_t append_block_to_chain(uint32_t* fat, DiskInfo* info, uint32_t chainHead) {
    //allocate a new block from the free list
    uint32_t freeListHead = get_list_head(info);
    if (freeListHead == FAT_EOF) return FAT_EOF; //no free blocks available
    uint32_t newBlock = freeListHead;
    freeListHead = fat[newBlock];
    fat[newBlock] = FAT_EOC;
    //find the last block in chain
    uint32_t cur = chainHead;
    while (fat[cur] != FAT_EOC) cur = fat[cur];
    //link the last block to the new block
    fat[cur] = newBlock;
    info->free_list_head = freeListHead;
    info->free_blocks--;
    return newBlock;
}

//deallocate a chain of blocks starting from 'start'
int deallocate_chain(uint32_t* fat, DiskInfo* info, uint32_t start) {
    uint32_t block = start;
    uint32_t freeListHead = get_list_head(info);
    while (block != FAT_EOC) {
        uint32_t next = fat[block];
        fat[block] = freeListHead;   //concatenate to free list
        freeListHead = block;        //new head of free list
        info->free_blocks++;
        if (next == FAT_EOC) break;
        block = next;
    }
    info->free_list_head = freeListHead;
    return 0;
}