#pragma once

#include "disk.h"
#include "fat.h"

#define ENTRY_TYPE_FILE 0
#define ENTRY_TYPE_DIR  1
#define MAX_DIR_ENTRIES 32

typedef struct {
    char name[MAX_NAME_LEN];
    uint8_t type;                           // ENTRY_TYPE_FILE or ENTRY_TYPE_DIR
    uint32_t size;                          // Size of the file (in bytes), for dirs it can be the number of entries
    uint32_t dir_blocks[MAX_DIR_ENTRIES];   // If it's a directory, array of starting blocks of contained files/dirs
    uint32_t parent_block;                  // parent directory block, EOF for root
    uint32_t current_block;                 // starting block
} Entry;

// Initialize an Entry structure
void init_directory(Entry* dir, const char* name, uint32_t start_block);

// Write an Entry to disk
int write_entry(void *disk_mem, const Entry* dir, size_t block_size, size_t disk_size_bytes);

// Read an Entry from disk into the provided Entry structure
Entry* read_directory(void *disk_mem, Entry* dir, size_t block_size, size_t disk_size_bytes);

// Read an Entry from disk given its block index
Entry* read_directory_from_block(void *disk_mem, uint32_t block_index, size_t block_size, size_t disk_size_bytes);

// Print the contents of an Entry
void print_directory(const Entry* dir);

// Update the children of a directory by adding a new child's starting block
void update_directory_children(Entry* dir, uint32_t child_start_block);

// Get the array of children blocks from a directory
uint32_t* get_children_blocks(const Entry* dir);

// Get the current path as a string by traversing up to the root
void get_current_path(char* disk_mem, uint32_t cursor, size_t block_size, size_t disk_size_bytes, char* out_path, size_t max_len);

//Initialize a file
void init_file(Entry* file, const char* name, uint32_t start_block);