#include "disk.h"
#include "shell.h"
#include "fat.h"
#include "entry.h"
#include "shell_commands.h"

void test_function() {
    //init disk
    const char* disk_path = "disk.img";

    //size_t disk_size = 32 * 1024 * 1024; // 32 MB
    size_t disk_size = 16 * 1024 * 1024; // 64 MB
    
    char* disk_memory = format_disk(disk_path, disk_size);
    if(disk_memory == NULL) handle_error("Failed to initialize disk");
    printf("Disk initialized successfully.\n");
    //read metainfo
    DiskInfo info;
    int res = read_metainfo(disk_memory, &info, BLOCK_SIZE, disk_size);
    if (res != 0) handle_error("Failed to read metainfo");
    print_disk_info(&info);
    //read fat
    uint32_t num_fat_entries = disk_size / BLOCK_SIZE;
    uint32_t fat[num_fat_entries];
    res = read_fat(disk_memory, fat, num_fat_entries, 1, BLOCK_SIZE, disk_size);
    if (res != 0) handle_error("Failed to read FAT");
    printf("FAT read successfully.\n");
    print_fat(fat, 10);

    //we know root is after metainfo and fat reserved blocks
    uint32_t reserved_blocks = calc_reserved_blocks(disk_size, BLOCK_SIZE);
    uint32_t root_block = reserved_blocks; // Assuming root is the first block after reserved
    //uint32_t cursor = root_block;
    //read root directory
    Entry* read_root = read_directory_from_block(disk_memory, root_block, BLOCK_SIZE, disk_size);
    if (read_root == NULL) handle_error("Failed to read root directory");
    printf("Root directory read successfully.\n");
    print_directory(read_root);

    uint32_t cursor = root_block;
    //create a new directory in root
    create_directory(disk_memory, "dir1", cursor, disk_size);
    
    // Clean up
    close_and_unmap_disk(disk_memory, disk_size);

}
int main() {
    test_function();
    //shell_init();

    return 0;
}