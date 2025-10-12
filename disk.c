#include "disk.h"
#include "fat.h"

#define ENTRIES_TO_PRINT 10

// print disk information
void print_disk_info(const DiskInfo* info) {
    printf("Name: %s\n", info->name);
    printf("Disk Size: %zu bytes\n", info->disk_size);
    printf("Block Size: %zu bytes\n", info->block_size);
    printf("Free Blocks: %zu\n", info->free_blocks);
    printf("Free List Head: %u\n", info->free_list_head);
}

// print disk status: metainfo and first ENTRIES_TO_PRINT FAT entries
void print_disk_status(char* disk_mem, size_t disk_size_bytes){
    //read metainfo
    printf("Disk status:\n");
    DiskInfo info = {0};
    int res = read_metainfo(disk_mem, &info, BLOCK_SIZE, disk_size_bytes);
    if (res != 0) handle_error("Failed to read metainfo");
    printf("Metainfo:\n");
    print_disk_info(&info);
    //read fat
    uint32_t num_fat_entries = disk_size_bytes / BLOCK_SIZE;
    uint32_t fat[num_fat_entries];
    res = read_fat(disk_mem, fat, num_fat_entries, 1, BLOCK_SIZE, disk_size_bytes);
    if (res != 0) handle_error("Failed to read FAT");
    printf("\n");
    printf("FAT (first %d entries):\n", ENTRIES_TO_PRINT);
    print_fat(fat, ENTRIES_TO_PRINT);
}

//initialize disk
char* open_and_map_disk(const char* filename, size_t filesize) {
    //syscall to open a file
    int fd = open(filename, O_RDWR | O_CREAT, 0666);
    if (fd == -1) {
        perror("Error opening/creating disk file");
        return NULL;
    }
    //set file size
    int result = ftruncate(fd, filesize);
    if (result == -1) {
        perror("Error setting disk file size");
        close(fd);
        return NULL;
    }
    //mmap
    char* file_memory = (char*) mmap(NULL, filesize, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    if (file_memory == MAP_FAILED) {
        perror("Error mapping disk file");
        close(fd);
        return NULL;
    }
    close(fd);
    return file_memory;
}

//compute blocks needed for metainfo + FAT
uint32_t calc_reserved_blocks(size_t disk_size, size_t block_size) {
    uint32_t num_blocks = disk_size / block_size;
    //fat blocks
    size_t fat_bytes = num_blocks * sizeof(uint32_t);
    uint32_t fat_blocks = (fat_bytes + block_size - 1) / block_size;
    // metainfo block: always one (the first block of the disk)
    uint32_t meta_blocks = 1;
    return meta_blocks + fat_blocks;
}

//read block from the disk to buffer
int read_block(char* disk_mem, uint32_t block_index, void *buffer, size_t block_size, size_t disk_size_bytes) {
    size_t offset = block_index * block_size;
    //prevent out of bounds access
    if (offset + block_size > disk_size_bytes) {
        return -1;
    }
    memcpy(buffer, (char*)disk_mem + offset, block_size);
    return 0;
}

//write block from buffer to the disk
int write_block(char* disk_mem, uint32_t block_index, const void *buffer, size_t block_size, size_t disk_size_bytes) {
    size_t offset = block_index * block_size;
    //prevent out of bounds access
    if (offset + block_size > disk_size_bytes) {
        return -1;
    }
    memcpy((char*)disk_mem + offset, buffer, block_size);
    //ensure persistence
    msync((char*)disk_mem + offset, block_size, MS_SYNC);
    return 0;
}

//close disk
void close_and_unmap_disk(char* file_memory, size_t filesize) {
    //sync changes to disk
    int result = msync(file_memory, filesize, MS_SYNC);
    if (result == -1) {
        perror("Error syncing disk file");
    }

    //unmap the file from memory
    result = munmap(file_memory, filesize);
    if (result == -1) {
        perror("Error unmapping disk file");
    }
}