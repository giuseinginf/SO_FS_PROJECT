#include "entry.h"

void init_directory(Entry* dir, const char* name, uint32_t start_block, uint8_t type){
    strncpy(dir->name, name, MAX_NAME_LEN);
    dir->type = type;
    dir->size = 1; // Initially empty
    memset(dir->dir_blocks, 0, sizeof(dir->dir_blocks));
    dir->current_block = start_block;
    dir->next_block = FAT_EOC; // No next block initially
}

int write_directory(void *disk_mem, const Entry* dir, size_t block_size, size_t disk_size_bytes){
    if (disk_mem == NULL || dir == NULL) {
        return -1; // Invalid parameters
    }
    // The entry contains the block where the directory should be written
    uint32_t block_index = dir->current_block;
    // Check bounds
    if (block_index * block_size >= disk_size_bytes) {
        return -1;
    }
    // Write the entry to the specified block
    char buffer[block_size];
    memset(buffer, 0, block_size);
    // Ensure dir is <= block_size
    if (sizeof(*dir) > block_size) {
        return -1; // Entry size exceeds block size
    }
    memcpy(buffer, dir, sizeof(Entry));
    return write_block(disk_mem, block_index, buffer, block_size, disk_size_bytes);
}

int read_directory(void *disk_mem, Entry* dir, size_t block_size, size_t disk_size_bytes){
    if (disk_mem == NULL || dir == NULL) {
        return -1; // Invalid parameters
    }
    uint32_t block_index = dir->current_block;
    if (block_index * block_size >= disk_size_bytes) {
        return -1;
    }
    char buffer[block_size];
    int res = read_block(disk_mem, block_index, buffer, block_size, disk_size_bytes);
    if (res != 0) return res;
    memcpy(dir, buffer, sizeof(Entry));
    return 0;
}

void print_directory(const Entry* dir){
    printf("Directory: %s\n", dir->name);
    printf("Type: %s\n", dir->type == ENTRY_TYPE_DIR ? "Directory" : "File");
    printf("Size: %u\n", dir->size);
    printf("Current Block: %u\n", dir->current_block);
    printf("Next Block: %u\n", dir->next_block);
    if (dir->type == ENTRY_TYPE_DIR) {
        printf("Directory Blocks: ");
        for (int i = 0; i < MAX_DIR_ENTRIES; i++) {
            if (dir->dir_blocks[i] != 0) {
                printf("%u ", dir->dir_blocks[i]);
            }
            else {
                if (i != 0) {
                    printf("No more directory blocks.\n");
                }
                else {
                    printf("No directory blocks.\n");
                }
                break; // Stop printing at the first empty entry
            }
        }
        printf("\n");
    }
}