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

typedef struct Disk {
    int        fd;          // file descriptor (>=0 if valid)
    uint8_t*   base;        // base address for mmap
    size_t     disk_size;  // total disk size
    uint32_t   block_size;  // logical block size (es. 4096)
    uint32_t   nblocks;     // == disk_size / block_size
} Disk;

// API

int  disk_init(Disk* d, const char* path, size_t disk_size, uint32_t block_size);

int disk_open(Disk* d, const char* path);

void*  disk_read(const Disk* d, uint32_t blkno);

void  disk_write(const Disk* d, uint32_t blkno, const void* in_buf);

int  disk_sync(const Disk* d);

int  disk_close(Disk* d);