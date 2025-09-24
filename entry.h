#pragma once

#include <stdint.h>
#include <stddef.h>

#define ENTRY_TYPE_FILE 0
#define ENTRY_TYPE_DIR  1
#define MAX_ENTRY_NAME  32
#define MAX_DIR_ENTRIES 32

typedef struct {
    char name[MAX_ENTRY_NAME];
    uint8_t type;         // ENTRY_TYPE_FILE or ENTRY_TYPE_DIR
    uint32_t size;        // Size of the file (in bytes), for dirs it can be the number of entries
    uint32_t dir_blocks[MAX_DIR_ENTRIES]; // If it's a directory, array of starting blocks of contained files/dirs    
} Entry;