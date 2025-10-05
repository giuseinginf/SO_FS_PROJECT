#include "fat.h"
#include "disk.h"
#include "entry.h"

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
void create_file(const char *name);

//rm
void remove_file(const char *name);

//cat
void concatenate_files(const char *file1, const char *file2);

//append
void append_to_file(const char *filename, const char *data);

//close
void close_filesystem();
