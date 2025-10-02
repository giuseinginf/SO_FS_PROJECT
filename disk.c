/*
Here we implement the disk API functions.
*/

#include "disk.h"

void handle_error(const char* msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

void print_disk_info(const DiskInfo* info) {
    printf("\n");
    printf("Disk Info:\n");
    printf("Name: %s\n", info->name);
    printf("Disk Size: %zu bytes\n", info->disk_size);
    printf("Block Size: %zu bytes\n", info->block_size);
    printf("Free Blocks: %zu\n", info->free_blocks);
    printf("Free List Head: %u\n", info->free_list_head);
    printf("\n");
}

//Initialize disk
char* open_and_map_disk(const char* filename, size_t filesize) {
    // syscall to open a file
    int fd = open(filename, O_RDWR | O_CREAT, 0666);
    if (fd == -1) {
        perror("Error opening/creating disk file");
        return NULL;
    }

    // set the file size
    int result = ftruncate(fd, filesize);
    if (result == -1) {
        perror("Error setting disk file size");
        close(fd);
        return NULL;
    }

    //Execute mmap
    char* file_memory = (char*) mmap(NULL,
                                     filesize,
                                     PROT_READ|PROT_WRITE,
                                     MAP_SHARED,
                                     fd,
                                     0);
    if (file_memory == MAP_FAILED) {
        perror("Error mapping disk file");
        close(fd);
        return NULL;
    }
    close(fd);
    return file_memory;
}

uint32_t calc_reserved_blocks(size_t disk_size, size_t block_size) {
    // total Number of data blocks on the disk
    uint32_t num_blocks = disk_size / block_size;

    // FAT space: one entry for each block, 4 bytes per entry
    size_t fat_bytes = num_blocks * sizeof(uint32_t);

    // How many blocks are needed for the FAT (round up)
    uint32_t fat_blocks = (fat_bytes + block_size - 1) / block_size;

    // Metainfo: 1 block reserved (can increase if structure grows)
    uint32_t meta_blocks = 1;

    // Total reserved blocks (metainfo + FAT)
    return meta_blocks + fat_blocks;
}

//read block
int read_block(char* disk_mem, uint32_t block_index, void *buffer, size_t block_size, size_t disk_size_bytes) {
    size_t offset = block_index * block_size;
    // Check that we don't go past the end of the disk
    if (offset + block_size > disk_size_bytes) {
        return -1; // Error: out of bounds
    }
    memcpy(buffer, (char*)disk_mem + offset, block_size);
    return 0; // Success
}

//write block
int write_block(char* disk_mem, uint32_t block_index, const void *buffer, size_t block_size, size_t disk_size_bytes) {
    size_t offset = block_index * block_size;
    if (offset + block_size > disk_size_bytes) {
        return -1; // Error: out of bounds
    }
    memcpy((char*)disk_mem + offset, buffer, block_size);
    msync((char*)disk_mem + offset, block_size, MS_SYNC); // Ensure persistence
    return 0;
}

//close disk
void close_and_unmap_disk(char* file_memory, size_t filesize) {
    // sync changes to disk
    int result = msync(file_memory, filesize, MS_SYNC);
    if (result == -1) {
        perror("Error syncing disk file");
    }

    // unmap the file from memory
    result = munmap(file_memory, filesize);
    if (result == -1) {
        perror("Error unmapping disk file");
    }
}