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

#define MAX_FILES 128
#define MAX_FILE_BLOCKS 64
#define MAX_NAME_LEN 32

// API

char* open_and_map_disk(const char* filename, size_t filesize);

uint32_t calc_reserved_blocks(size_t disk_size, size_t block_size);

int read_block(void *disk_mem, uint32_t block_index, void *buffer, size_t block_size, size_t disk_size_bytes);

int write_block(void *disk_mem, uint32_t block_index, const void *buffer, size_t block_size, size_t disk_size_bytes);

void close_and_unmap_disk(char* file_memory, size_t filesize);
