/*
Here we implement the disk API functions.
*/

#include "disk.h"

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

//read block
int read_block(void *disk_mem, uint32_t block_index, void *buffer, size_t block_size, size_t disk_size_bytes) {
    size_t offset = block_index * block_size;
    // Check that we don't go past the end of the disk
    if (offset + block_size > disk_size_bytes) {
        return -1; // Error: out of bounds
    }
    memcpy(buffer, (char*)disk_mem + offset, block_size);
    return 0; // Success
}

//write block
int write_block(void *disk_mem, uint32_t block_index, const void *buffer, size_t block_size, size_t disk_size_bytes) {
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