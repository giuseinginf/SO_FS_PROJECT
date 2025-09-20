#include "disk.h"
#include "shell.h"
#include "fat.h"


void test_function() {
    //init disk
    const char* disk_path = "virtual_disk.img";

    //size_t disk_size = 32 * 1024 * 1024; // 32 MB
    size_t disk_size = 16 * 1024 * 1024; // 16 MB
    uint32_t block_size = 4096; // 4 KB

    char* disk_memory = open_and_map_disk(disk_path, disk_size);
    if(!disk_memory) {
        perror("Failed to initialize disk");
        exit(EXIT_FAILURE);
    }
    printf("Disk initialized successfully. Name: %s, size: %zu bytes, blocks: %u bytes\n", disk_path, disk_size, block_size);

    uint32_t reserved_blocks = calc_reserved_blocks(disk_size, block_size);
    printf("Reserved blocks: %u\n", reserved_blocks);

    //fat & metainfo test
    DiskInfo info = {0};
    info.block_size = block_size;
    info.disk_size = disk_size;
    info.free_blocks = (disk_size / block_size) - reserved_blocks;
    info.free_list_head = reserved_blocks; // First free block after reserved blocks
    snprintf(info.name, MAX_NAME_LEN, "virtual_disk.img");
    if(write_metainfo(disk_memory, &info, block_size, disk_size) != 0) {
        perror("Failed to write metainfo");
        close_and_unmap_disk(disk_memory, disk_size);
        exit(EXIT_FAILURE);
    }
    printf("Metainfo written successfully.\n");

    DiskInfo read_info = {0};
    if(read_metainfo(disk_memory, &read_info, block_size, disk_size) != 0) {
        perror("Failed to read metainfo");
        close_and_unmap_disk(disk_memory, disk_size);
        exit(EXIT_FAILURE);
    }
    printf("Metainfo read successfully. Name: %s, size: %zu bytes, block size: %zu bytes, free blocks: %zu, free list head: %u\n",
           read_info.name, read_info.disk_size, read_info.block_size, read_info.free_blocks, read_info.free_list_head);
    
    // Initialize FAT
    uint32_t num_blocks = disk_size / block_size;
    uint32_t num_fat_entries = num_blocks;
    uint32_t* fat = (uint32_t*) calloc(num_fat_entries, sizeof(uint32_t));
    if (!fat) {
        perror("Failed to allocate FAT");
        close_and_unmap_disk(disk_memory, disk_size);
        exit(EXIT_FAILURE); 
    }
    //each fat block points to the next one, last block is EOF
    for (uint32_t i = 0; i < num_fat_entries; i++) {
        if (i < num_fat_entries - 1) {
            fat[i] = i + 1; // Next block
        } else {
            fat[i] = FAT_EOF; // End of file marker
        }
    }
    // Write FAT to disk
    uint32_t fat_start_block = 1; // Assuming metainfo occupies block 0
    if (write_fat(disk_memory, fat, num_fat_entries, fat_start_block, block_size, disk_size) != 0) {
        perror("Failed to write FAT");
        free(fat);
        close_and_unmap_disk(disk_memory, disk_size);
        exit(EXIT_FAILURE);
    }
    printf("FAT written successfully.\n");

    free(fat);

    //read some FAT entries
    uint32_t* read_fat_entries = (uint32_t*) calloc(num_fat_entries, sizeof(uint32_t));
    if (!read_fat_entries) {
        perror("Failed to allocate memory for reading FAT");
        close_and_unmap_disk(disk_memory, disk_size);
        exit(EXIT_FAILURE);
    }
    //read fat
    if (read_fat(disk_memory, read_fat_entries, num_fat_entries, fat_start_block, block_size, disk_size) != 0) {
        perror("Failed to read FAT");
        free(read_fat_entries);
        close_and_unmap_disk(disk_memory, disk_size);
        exit(EXIT_FAILURE);
    }
    printf("FAT read successfully. First 10 entries:\n");
    for (uint32_t i = 0; i < 10 && i < num_fat_entries; i++) {
        printf("FAT[%u] = %u\n", i, read_fat_entries[i]);
    }
    free(read_fat_entries);

        // Clean up
    close_and_unmap_disk(disk_memory, disk_size);
}

int main() {
    test_function();
//    shell_init();

    return 0;
}