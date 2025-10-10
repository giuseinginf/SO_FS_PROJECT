#pragma once

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <stdbool.h>

#define BLOCK_SIZE 4096 // 4 KB = 4096 bytes

#define MAX_FILES 128
#define MAX_FILE_BLOCKS 64
#define MAX_NAME_LEN 32

typedef struct{
    char name[MAX_NAME_LEN];    // disk file name
    size_t disk_size;           // total disk size in bytes
    size_t block_size;          // block size (e.g., 4096)
    size_t free_blocks;         // free blocks
    uint32_t free_list_head;    // index of the first free block
} DiskInfo;

// Error handling function
void handle_error(const char* msg);

// Print disk information: name, size, block size, free blocks, free list head
void print_disk_info(const DiskInfo* info);

// Print disk status: metainfo and first 10 FAT entries
void print_disk_status(char* disk_mem, size_t disk_size_bytes);

//Initialize disk
char* open_and_map_disk(const char* filename, size_t filesize);

//Compute number of reserved blocks (metainfo + FAT)
uint32_t calc_reserved_blocks(size_t disk_size, size_t block_size);

// Read a block from disk into buffer
int read_block(char* disk_mem, uint32_t block_index, void *buffer, size_t block_size, size_t disk_size_bytes);

// Write a block from buffer to disk
int write_block(char* disk_mem, uint32_t block_index, const void *buffer, size_t block_size, size_t disk_size_bytes);

// Unmap and close the disk
void close_and_unmap_disk(char* file_memory, size_t filesize);
