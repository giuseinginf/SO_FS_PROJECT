#include "disk.h"
#include "shell.h"
#include "fat.h"

void printDiskInfo(const DiskInfo* info) {
    printf("\n");
    printf("Disk Info:\n");
    printf("Name: %s\n", info->name);
    printf("Disk Size: %zu bytes\n", info->disk_size);
    printf("Block Size: %zu bytes\n", info->block_size);
    printf("Free Blocks: %zu\n", info->free_blocks);
    printf("Free List Head: %u\n", info->free_list_head);
    printf("\n");
}

void test_function() {
    //init disk
    const char* disk_path = "virtual_disk.img";

    //size_t disk_size = 32 * 1024 * 1024; // 32 MB
    size_t disk_size = 16 * 1024 * 1024; // 16 MB
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
    snprintf(info.name, MAX_NAME_LEN, "virtual_disk.img");
    
    uint32_t num_blocks = disk_size / block_size;
    uint32_t num_fat_entries = num_blocks;
    
    uint32_t fat[num_fat_entries]; // Array to hold FAT entries
    init_fat(fat, num_fat_entries);
    printf("FAT initialized successfully.\n");

    //print metainfo
    printDiskInfo(&info);
    
    printFat(fat, 10);
    
    //we allocate the metainfo block
    int res = allocateBlock(fat, &info);
    if (res == -1) {
        handle_error("Failed to allocate block for metainfo");
    }
    printf("Reserved block %d allocated for metainfo.\n", res);
    //we append the reserved blocks to the chain
    uint32_t index = 0;
    for (uint32_t i = 1; i < reserved_blocks; i++) {
        int new_block = appendBlockToChain(fat, &info, index);
        if (new_block == -1) {
            handle_error("Failed to append reserved block to chain");
        }
        index++;
    }

    printDiskInfo(&info);

    printFat(fat, 10);

    //deallocate the reserved blocks chain
    deallocateChain(fat, &info, 0);
    printf("Reserved blocks chain deallocated.\n");

    printDiskInfo(&info);
    printFat(fat, 10);
    
    // Clean up
    close_and_unmap_disk(disk_memory, disk_size);
}

int main() {
    test_function();
//  shell_init();

    return 0;
}