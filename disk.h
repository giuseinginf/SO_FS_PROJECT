#pragma once

#include "utils.h"

#define BLOCK_SIZE 4096  //4 KB

#define MAX_FILES 128
#define MAX_FILE_BLOCKS 64
#define MAX_NAME_LEN 32

typedef struct{
    char name[MAX_NAME_LEN];    // disk file name
    size_t disk_size;           // total disk size in bytes
    size_t block_size;         // block size
    size_t free_blocks;        // free blocks
    uint32_t free_list_head;   // index of the first free block
} DiskInfo;

//print disk information
void print_disk_info(const DiskInfo* info);

//print disk status
void print_disk_status(char* disk_mem, size_t disk_size_bytes);

//initialize disk
char* open_and_map_disk(const char* filename, size_t filesize);

//compute number of reserved blocks (metainfo + FAT)
uint32_t calc_reserved_blocks(size_t disk_size, size_t block_size);

//read a block from disk into buffer
int read_block(char* disk_mem, uint32_t block_index, void *buffer, size_t block_size, size_t disk_size_bytes);

//write a block from buffer to disk
int write_block(char* disk_mem, uint32_t block_index, const void *buffer, size_t block_size, size_t disk_size_bytes);

//unmap and close the disk
void close_and_unmap_disk(char* file_memory, size_t filesize);
