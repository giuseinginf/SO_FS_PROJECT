/*
Here we define the Disk structure and its associated functions.
*/

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

#define FAT_SIZE 1024 * 1024 // 1 MB
#define FAT_NUM_BLOCKS (FAT_SIZE / BLOCK_SIZE) // 256

#define MAX_FILES 128
#define MAX_FILE_BLOCKS 64

typedef struct Disk {
    int        fd;          // file descriptor (>=0 if valid)
    uint8_t*   base;        // base address for mmap
    size_t     disk_size;  // total disk size
    uint32_t   block_size;  // logical block size (= BLOCK_SIZE)
    uint32_t   nblocks;     // 
} Disk;

// API

Disk* disk_init(const char* path, size_t disk_size, uint32_t block_size);

void disk_open(Disk* d, const char* path);

//read one block
void*  disk_read(const Disk* d, uint32_t blkno);

//read more blocks
void*  disk_read_blocks(const Disk* d, uint32_t blkno, uint32_t blocks);

//write one block
void  disk_write(const Disk* d, uint32_t blkno, const void* in_buf);

//write more blocks
void disk_write_blocks(const Disk* d, uint32_t blkno, const void* in_buf, uint32_t blocks);

int  disk_sync(const Disk* d);

void  disk_close(Disk* d);