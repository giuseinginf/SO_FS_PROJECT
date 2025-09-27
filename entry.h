#pragma once

#include "disk.h"
#include "fat.h"

#define ENTRY_TYPE_FILE 0
#define ENTRY_TYPE_DIR  1
#define MAX_DIR_ENTRIES 32

typedef struct {
    char name[MAX_NAME_LEN];
    uint8_t type;         // ENTRY_TYPE_FILE or ENTRY_TYPE_DIR
    uint32_t size;        // Size of the file (in bytes), for dirs it can be the number of entries
    uint32_t dir_blocks[MAX_DIR_ENTRIES]; // If it's a directory, array of starting blocks of contained files/dirs
    uint32_t current_block; // starting block
    uint32_t next_block;  // next block of the entry, FAT_EOC if none
} Entry;

void init_directory(Entry* dir, const char* name, uint32_t start_block, uint8_t type);

int write_directory(void *disk_mem, const Entry* dir, size_t block_size, size_t disk_size_bytes);

int read_directory(void *disk_mem, Entry* dir, size_t block_size, size_t disk_size_bytes);

void print_directory(const Entry* dir);