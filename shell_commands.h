#pragma once

#include "fat.h"
#include "disk.h"
#include "entry.h"
#include "utils.h"

//format
char* format_disk(const char *filename, size_t size);

//mkdir
void create_directory(char* disk_mem, const char *name, uint32_t parent_block, size_t disk_size_bytes);

//rmdir
void remove_directory(char* disk_mem, const char *name, uint32_t parent_block, size_t disk_size_bytes);

//ls
void list_directory_contents();

//cd
uint32_t change_directory(const char *path, uint32_t cursor, char* disk_mem, size_t disk_size_bytes);

//touch
void create_file(char* disk_mem, const char* name, uint32_t parent_block, size_t disk_size_bytes);

//rm
void remove_file(char* disk_mem, const char *name, uint32_t parent_block, size_t disk_size_bytes);

//append
void append_to_file(char* disk_mem, char* data, size_t data_len, char* filename, uint32_t cursor, size_t block_size, size_t disk_size_bytes);

//cat
void cat_file(char* disk_mem, const char* filename, uint32_t cursor, size_t block_size, size_t disk_size_bytes);