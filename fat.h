#pragma once

#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "disk.h"

#define FAT_EOC 0xFFFFFFFF  //marks last block of a file
#define FAT_EOF 0xFFFFFFFE  //marks end of FAT itself
#define MAX_NAME_LEN 32

typedef struct{
    char name[MAX_NAME_LEN];    // nome file disco
    size_t disk_size;           // dimensione totale disco in byte
    size_t block_size;          // dimensione blocco (es: 4096)
    size_t free_blocks;         // blocchi liberi
    uint32_t free_list_head;    // indice del primo blocco libero
} DiskInfo;

int write_metainfo(void *disk_mem, const DiskInfo *info, size_t block_size, size_t disk_size_bytes);

int read_metainfo(void *disk_mem, DiskInfo *info, size_t block_size, size_t disk_size_bytes);

void init_fat(uint32_t* fat, uint32_t num_entries);

void printFat(const uint32_t* fat, uint32_t num_entries);

int write_fat(void *disk_mem, const uint32_t *fat, uint32_t num_fat_entries, uint32_t start_block, size_t block_size, size_t disk_size_bytes);

int read_fat(void *disk_mem, uint32_t *fat, uint32_t num_fat_entries, uint32_t start_block, size_t block_size, size_t disk_size_bytes);

uint32_t get_free_block(const DiskInfo* info);

int allocateBlock(uint32_t* fat, DiskInfo* info);

int appendBlockToChain(uint32_t* fat, DiskInfo* info, uint32_t chainHead);

int deallocateChain(uint32_t* fat, DiskInfo* info, uint32_t start);