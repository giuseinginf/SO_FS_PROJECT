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
    
    //read disk status
    print_disk_status(disk_memory, disk_size);

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
    printf("Cursor at root block: %u\n", cursor);
    //create a new directory in root
    create_directory(disk_memory, "dir1", cursor, disk_size);

    //we read root again to see if dir1 is there
    printf("Reading the root again...\n");
    Entry* read_root_again = read_directory_from_block(disk_memory, root_block, BLOCK_SIZE, disk_size);
    if (read_root_again == NULL) handle_error("Failed to read root directory");
    print_directory(read_root_again);
    //fat and metainfo should be updated already by create_directory. We can print them again
    
    //we read status again
    print_disk_status(disk_memory, disk_size);

    //we now print with list_directory_contents
    printf("Listing contents of root directory:\n");
    list_directory_contents(disk_memory, cursor, disk_size);
    
    // Clean up
    close_and_unmap_disk(disk_memory, disk_size);

    printf("Disk reopened.\n");

    //we reopen the disk to check if dir1 is there
    disk_memory = open_and_map_disk(disk_path, disk_size);
    if(disk_memory == NULL) handle_error("Failed to reopen disk");

    //read status again
    print_disk_status(disk_memory, disk_size);

    //read root directory again
    Entry* new_read_root = read_directory_from_block(disk_memory, root_block, BLOCK_SIZE, disk_size);
    if (new_read_root == NULL) handle_error("Failed to read root directory");
    printf("Root directory read successfully.\n");
    print_directory(new_read_root);

    printf("All read.\n");

}
int main() {
    test_function();
    //shell_init();

    return 0;
}