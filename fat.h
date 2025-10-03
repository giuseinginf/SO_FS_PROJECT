#pragma once

#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "disk.h"

#define FAT_EOC 0xFFFFFFFF  //marks last block of a file
#define FAT_EOF 0xFFFFFFFE  //marks end of FAT itself

int write_metainfo(char* disk_mem, const DiskInfo *info, size_t block_size, size_t disk_size_bytes);

int read_metainfo(char* disk_mem, DiskInfo *info, size_t block_size, size_t disk_size_bytes);

void init_fat(uint32_t* fat, uint32_t num_entries);

void print_fat(const uint32_t* fat, uint32_t num_entries);

void load_info_and_fat(char* disk_mem, DiskInfo* info, uint32_t* fat, size_t disk_size);

int update_fat_and_metainfo(char* disk_mem, uint32_t *fat, uint32_t num_fat_entries, uint32_t fat_start_block, DiskInfo *info, size_t block_size, size_t disk_size_bytes);

int write_fat(char* disk_mem, const uint32_t *fat, uint32_t num_fat_entries, uint32_t start_block, size_t block_size, size_t disk_size_bytes);

int read_fat(char* disk_mem, uint32_t *fat, uint32_t num_fat_entries, uint32_t start_block, size_t block_size, size_t disk_size_bytes);

uint32_t get_free_block(const DiskInfo* info);

uint32_t allocateBlock(uint32_t* fat, DiskInfo* info);

uint32_t appendBlockToChain(uint32_t* fat, DiskInfo* info, uint32_t chainHead);

int deallocateChain(uint32_t* fat, DiskInfo* info, uint32_t start);