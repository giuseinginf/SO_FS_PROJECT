#pragma once

#include "disk.h"
#include "../utils/utils.h"

#define FAT_EOC 0xFFFFFFFF  //marks last block of a file
#define FAT_EOF 0xFFFFFFFE  //marks end of FAT itself

//write metainfo to disk
int write_metainfo(char* disk_mem, const DiskInfo *info, size_t block_size, size_t disk_size_bytes);

//read metainfo from disk
int read_metainfo(char* disk_mem, DiskInfo *info, size_t block_size, size_t disk_size_bytes);

//initialize FAT struct
void init_fat(uint32_t* fat, uint32_t num_entries);

//print FAT entries
void print_fat(const uint32_t* fat, uint32_t num_entries);

//load metainfo and FAT from disk into RAM
void read_info_and_fat(char* disk_mem, DiskInfo* info, uint32_t* fat, size_t disk_size);

//update FAT and metainfo on disk
int write_info_and_fat(char* disk_mem, uint32_t *fat, uint32_t num_fat_entries, uint32_t fat_start_block, DiskInfo *info, size_t block_size, size_t disk_size_bytes);

//write FAT to disk
int write_fat(char* disk_mem, const uint32_t *fat, uint32_t num_fat_entries, uint32_t start_block, size_t block_size, size_t disk_size_bytes);

//read FAT from disk
int read_fat(char* disk_mem, uint32_t *fat, uint32_t num_fat_entries, uint32_t start_block, size_t block_size, size_t disk_size_bytes);

//get the index of the first free block from metainfo
uint32_t get_list_head(const DiskInfo* info);

//allocate a block from the free list and update metainfo and FAT
uint32_t allocate_block(uint32_t* fat, DiskInfo* info);

//append a new block to the end of a file's block chain
uint32_t append_block_to_chain(uint32_t* fat, DiskInfo* info, uint32_t chainHead);

//deallocate a chain of blocks starting from 'start'
int deallocate_chain(uint32_t* fat, DiskInfo* info, uint32_t start);