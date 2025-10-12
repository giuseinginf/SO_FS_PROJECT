#pragma once

#include "disk.h"
#include "fat.h"
#include "utils.h"

#define ENTRY_TYPE_FILE 0
#define ENTRY_TYPE_DIR  1
#define MAX_DIR_ENTRIES 32

typedef struct {
    char name[MAX_NAME_LEN];
    uint8_t type;                           //entries can be ENTRY_TYPE_FILE or ENTRY_TYPE_DIR
    uint32_t size;                          //size of the entry
    uint32_t dir_blocks[MAX_DIR_ENTRIES];   //if it's a directory, the array contains the starting blocks of contained files/dirs
    uint32_t parent_block;                  //parent directory block, EOF for root
    uint32_t current_block;                 //starting block
} Entry;

//initialize an Entry structure
void init_directory(Entry* dir, const char* name, uint32_t start_block);

//write an Entry to disk
int write_entry(void *disk_mem, const Entry* dir, size_t block_size, size_t disk_size_bytes);

//read an Entry from disk into the provided Entry structure
Entry* read_directory(void *disk_mem, Entry* dir, size_t block_size, size_t disk_size_bytes);

//read an Entry from disk given its block index
Entry* read_directory_from_block(void *disk_mem, uint32_t block_index, size_t block_size, size_t disk_size_bytes);

//print the contents of an Entry
void print_directory(const Entry* dir);

//update the children of a directory by adding a new child's starting block
void update_directory_children(Entry* dir, uint32_t child_start_block);

//get the array of children blocks from a directory
uint32_t* get_children_blocks(const Entry* dir);

//get the current path as a string by traversing up to the root
void get_current_path(char* disk_mem, uint32_t cursor, size_t block_size, size_t disk_size_bytes, char* out_path, size_t max_len);

//initialize a file
void init_file(Entry* file, const char* name, uint32_t start_block);