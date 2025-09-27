#include "disk.h"
#include "shell.h"
#include "fat.h"
#include "entry.h"

void test_function() {
    //init disk
    const char* disk_path = "disk.img";

    //size_t disk_size = 32 * 1024 * 1024; // 32 MB
    size_t disk_size = 16 * 1024 * 1024; // 64 MB
    uint32_t block_size = 4096; // 4 KB

    char* disk_memory = open_and_map_disk(disk_path, disk_size);
    if(disk_memory == NULL){
        handle_error("Failed to initialize disk");
    }
    printf("Disk initialized successfully.\n");

    uint32_t reserved_blocks = calc_reserved_blocks(disk_size, block_size);
    printf("Reserved blocks: %u\n", reserved_blocks);
    uint32_t metainfo_blocks = 1;
    uint32_t fat_blocks = reserved_blocks - metainfo_blocks;
    printf("Metainfo blocks: %u, FAT blocks: %u\n", metainfo_blocks, fat_blocks);

    //fat & metainfo test
    DiskInfo info = {0};
    info.block_size = block_size;
    info.disk_size = disk_size;
    info.free_blocks = (disk_size / block_size);
    info.free_list_head = 0; // First free block after reserved blocks
    snprintf(info.name, MAX_NAME_LEN, "disk.img");
    
    uint32_t num_blocks = disk_size / block_size;
    uint32_t num_fat_entries = num_blocks;
    
    uint32_t fat[num_fat_entries]; // Array to hold FAT entries
    init_fat(fat, num_fat_entries);
    printf("FAT initialized successfully.\n");

    /*
    //print metainfo
    printDiskInfo(&info);
    printFat(fat, 10);
    */
    
    //we allocate the metainfo block
    uint32_t result = allocateBlock(fat, &info);
    if (result == FAT_EOF) {
        handle_error("Failed to allocate block for metainfo");
    }
    printf("Reserved block %d allocated for metainfo.\n", result);
    //we append the reserved blocks to the chain
    uint32_t index = 0;
    for (uint32_t i = 1; i < reserved_blocks; i++) {
        int new_block = appendBlockToChain(fat, &info, index);
        if (new_block == -1) {
            handle_error("Failed to append reserved block to chain");
        }
        index++;
    }

    /*
    printDiskInfo(&info);
    printFat(fat, 10);
    */


    //write metainfo and fat to disk
    int res = write_metainfo(disk_memory, &info, block_size, disk_size);
    if (res != 0) {
        handle_error("Failed to write metainfo to disk");
    }
    printf("Metainfo written to disk successfully.\n");
    res = write_fat(disk_memory, fat, num_fat_entries, 1, block_size, disk_size);
    if (res != 0) {
        handle_error("Failed to write FAT to disk");
    }
    printf("FAT written to disk successfully.\n");

    //read back metainfo and fat
    DiskInfo read_info = {0};
    read_metainfo(disk_memory, &read_info, block_size, disk_size);
    printf("Metainfo read from disk successfully.\n");
    printDiskInfo(&read_info);
    uint32_t read_fat_array[num_fat_entries];
    res = read_fat(disk_memory, read_fat_array, num_fat_entries, 1, block_size, disk_size);
    if (res != 0) {
        handle_error("Failed to read FAT from disk");
    }
    printf("FAT read from disk successfully.\n");
    printFat(read_fat_array, 10);

    //add root directory
    //allocate block for root
    result = allocateBlock(read_fat_array, &read_info);
    if (result == FAT_EOF) {
        handle_error("Failed to allocate block for root directory");
    }

    //allocate root block
    Entry root = {0};
    init_directory(&root, "/", result, ENTRY_TYPE_DIR);
    printf("Root directory initialized successfully.\n");
    print_directory(&root);

    //write root to disk
    res = write_directory(disk_memory, &root, block_size, disk_size);
    if (res != 0) {
        handle_error("Failed to write root directory to disk");
    }
    printf("Root directory written to disk successfully.\n");

    //update fat and metainfo
    res = update_fat_and_metainfo(disk_memory, read_fat_array, num_fat_entries, 1, &read_info, block_size, disk_size);
    if (res != 0) { 
        handle_error("Failed to update FAT and metainfo on disk");
    }
    printf("FAT and metainfo updated on disk successfully.\n");

    //print free list head
    printf("Free list head after root allocation: %u\n", read_info.free_list_head);

    //print root directory
    print_directory(&root);

    //print fat
    printFat(read_fat_array, 10);

    // Clean up
    close_and_unmap_disk(disk_memory, disk_size);

}
int main() {
    test_function();
    //shell_init();

    return 0;
}