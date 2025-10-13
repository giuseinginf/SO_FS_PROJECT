#include "entry.h"

//initialize an Entry structure
void init_directory(Entry* dir, const char* name, uint32_t start_block){
    strncpy(dir->name, name, MAX_NAME_LEN);
    dir->type = ENTRY_TYPE_DIR;
    dir->size = 0; //initially empty
    memset(dir->dir_blocks, 0, sizeof(dir->dir_blocks));
    dir->parent_block = FAT_EOF; //no parent initially
    dir->current_block = start_block;
}

//write an Entry to disk
int write_entry(void *disk_mem, const Entry* dir, size_t block_size, size_t disk_size_bytes){
    if (disk_mem == NULL || dir == NULL) return -1; //invalid parameters
    //the entry contains the block where the directory should be written
    uint32_t block_index = dir->current_block;
    //check bounds
    if (block_index * block_size >= disk_size_bytes) return -1;
    //write the entry to the specified block
    char buffer[block_size];
    memset(buffer, 0, block_size);
    if (sizeof(*dir) > block_size) return -1; // Entry size exceeds block size
    memcpy(buffer, dir, sizeof(Entry));
    return write_block(disk_mem, block_index, buffer, block_size, disk_size_bytes);
}

//read an Entry from disk into the provided Entry structure
Entry* read_directory(void *disk_mem, Entry* dir, size_t block_size, size_t disk_size_bytes){
    if (disk_mem == NULL || dir == NULL) return NULL; //invalid parameters
    uint32_t block_index = dir->current_block;
    if (block_index * block_size >= disk_size_bytes) return NULL; //out of bounds
    char buffer[block_size];
    int res = read_block(disk_mem, block_index, buffer, block_size, disk_size_bytes);
    if (res != 0) return NULL;
    memcpy(dir, buffer, sizeof(Entry));
    return dir;
}

//read an Entry from disk given its block index
Entry* read_directory_from_block(void *disk_mem, uint32_t block_index, size_t block_size, size_t disk_size_bytes){
    if (disk_mem == NULL) return NULL; //invalid parameters
    if (block_index * block_size >= disk_size_bytes) return NULL; //out of bounds
    char buffer[block_size];
    int res = read_block(disk_mem, block_index, buffer, block_size, disk_size_bytes);
    if (res != 0) return NULL;
    Entry* dir = malloc(sizeof(Entry));
    if (dir == NULL) return NULL;
    memcpy(dir, buffer, sizeof(Entry));
    return dir;
}

//print the contents of an Entry
void print_directory(const Entry* dir){
    printf("Directory: %s\n", dir->name);
    printf("Type: %s\n", dir->type == ENTRY_TYPE_DIR ? "Directory" : "File");
    printf("Size: %u\n", dir->size);
    printf("Parent Block: %s\n", dir->parent_block == FAT_EOF ? "None" : "");
    printf("Current Block: %u\n", dir->current_block);
    //only for directories, print the indexes of children blocks
    if (dir->type == ENTRY_TYPE_DIR) {
        printf("Directory Blocks: ");
        int has_child = 0;
        for (int i = 0; i < MAX_DIR_ENTRIES; i++) {
            if (dir->dir_blocks[i] != 0) {
                printf("%u ", dir->dir_blocks[i]);
                has_child = 1;
            }
        }
        if (has_child) {
            printf("-\n");
        } else {
            printf("No directory blocks.\n");
        }
    }
}

//update the children of a directory by adding a new child's starting block
void update_directory_children(Entry* dir, uint32_t child_start_block){
    //scan the dir_blocks array for the first 0 entry and then add the new child's start block there
    if (dir == NULL) return;
    for (int i = 0; i < MAX_DIR_ENTRIES; i++) {
        if (dir->dir_blocks[i] == 0) {
            dir->dir_blocks[i] = child_start_block;
            return;
        }
    }
    //directory is full
    printf("Directory is full, cannot add more children.\n");
}

//get the array of children blocks from a directory
uint32_t* get_children_blocks(const Entry* dir) {
    return (uint32_t*)dir->dir_blocks;
}

//constructs the current path as a string by traversing up the directory tree from the given cursor block.
void get_current_path(char* disk_mem, uint32_t cursor, size_t block_size, size_t disk_size_bytes, char* out_path, size_t max_len) {
    //out_path has to be already allocated, max_len is the maximum size
    char temp[MAX_NAME_LEN * MAX_DIR_ENTRIES] = {0};
    int pos = sizeof(temp) - 1;
    temp[pos] = '\0';
    uint32_t block = cursor;
    //traverse up to root
    while (block != FAT_EOF) {
        Entry* dir = read_directory_from_block(disk_mem, block, block_size, disk_size_bytes);
        if (dir == NULL) break;
        size_t name_len = strlen(dir->name);
        if (strcmp(dir->name, "/") != 0 && name_len > 0) {
            pos -= name_len;
            if (pos < 0) break;
            memcpy(&temp[pos], dir->name, name_len);
            pos--;
            if (pos < 0) break;
            temp[pos] = '/';
        }
        block = dir->parent_block;
        free(dir);
    }
    //copy the result to out_path
    if (pos < 0) pos = 0;
    strncpy(out_path, &temp[pos], max_len);
    out_path[max_len - 1] = '\0';
    if (strlen(out_path) == 0) {
        //if the cursor is at root, path is "/"
        strncpy(out_path, "/", max_len);
        out_path[max_len - 1] = '\0';
    }
}

//initialize a file
void init_file(Entry* file, const char* name, uint32_t start_block){
    strncpy(file->name, name, MAX_NAME_LEN);
    file->type = ENTRY_TYPE_FILE;
    file->size = 0; //initially empty
    memset(file->dir_blocks, 0, sizeof(file->dir_blocks));
    file->parent_block = FAT_EOF; //no parent initially
    file->current_block = start_block;
}