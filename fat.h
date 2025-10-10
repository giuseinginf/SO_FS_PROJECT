#pragma once

#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "disk.h"

#define FAT_EOC 0xFFFFFFFF  //marks last block of a file
#define FAT_EOF 0xFFFFFFFE  //marks end of FAT itself

// Write metainfo to disk
int write_metainfo(char* disk_mem, const DiskInfo *info, size_t block_size, size_t disk_size_bytes);

// Read metainfo from disk
int read_metainfo(char* disk_mem, DiskInfo *info, size_t block_size, size_t disk_size_bytes);

//initialize FAT struct
void init_fat(uint32_t* fat, uint32_t num_entries);

// Print FAT entries
void print_fat(const uint32_t* fat, uint32_t num_entries);

// Load metainfo and FAT from disk into RAM
void read_info_and_fat(char* disk_mem, DiskInfo* info, uint32_t* fat, size_t disk_size);

// Update FAT and metainfo on disk
int write_info_and_fat(char* disk_mem, uint32_t *fat, uint32_t num_fat_entries, uint32_t fat_start_block, DiskInfo *info, size_t block_size, size_t disk_size_bytes);

// Write FAT to disk
int write_fat(char* disk_mem, const uint32_t *fat, uint32_t num_fat_entries, uint32_t start_block, size_t block_size, size_t disk_size_bytes);

// Read FAT from disk
int read_fat(char* disk_mem, uint32_t *fat, uint32_t num_fat_entries, uint32_t start_block, size_t block_size, size_t disk_size_bytes);

// Get the index of the first free block from metainfo
uint32_t get_list_head(const DiskInfo* info);

// Allocate a block from the free list and update metainfo and FAT
uint32_t allocate_block(uint32_t* fat, DiskInfo* info);

// Append a new block to the end of a file's block chain
uint32_t append_block_to_chain(uint32_t* fat, DiskInfo* info, uint32_t chainHead);

// Deallocate a chain of blocks starting from 'start'
int deallocate_chain(uint32_t* fat, DiskInfo* info, uint32_t start);