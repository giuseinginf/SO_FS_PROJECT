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

    //format disk
    char* disk_memory = format_disk(disk_path, disk_size);
    if(disk_memory == NULL) handle_error("Failed to initialize disk");
    
    printf("Disk initialized successfully.\n");
    
    //read disk status
    printf("\n");
    print_disk_status(disk_memory, disk_size);
    printf("\n");

    //we know root is after metainfo and fat reserved blocks
    uint32_t reserved_blocks = calc_reserved_blocks(disk_size, BLOCK_SIZE);
    uint32_t root_block = reserved_blocks; // Assuming root is the first block after reserved
    //uint32_t cursor = root_block;
    //read root directory
    Entry* read_root = read_directory_from_block(disk_memory, root_block, BLOCK_SIZE, disk_size);
    //we check if root directory is valid
    if (read_root == NULL) handle_error("Failed to read root directory");

    //printf("Root directory read successfully.\n");
    printf("Root directory:\n");
    print_directory(read_root);
    free(read_root);

    uint32_t cursor = root_block;
    printf("Cursor at root block: %u\n", cursor);
    printf("\n");

    //we add a new file inside root
    create_file(disk_memory, "file1.txt", cursor, disk_size);
    printf("\n");

    //we add a new directory inside root
    create_directory(disk_memory, "dir1", cursor, disk_size);
    printf("\n");

    //we list the contents of root
    printf("Contents of root directory after adding file1.txt:\n");
    list_directory_contents(disk_memory, cursor, disk_size);
    printf("\n");

    //we print the updated root directory
    Entry* updated_root = read_directory_from_block(disk_memory, root_block, BLOCK_SIZE, disk_size);
    if (updated_root == NULL) handle_error("Failed to read updated root directory");
    printf("Updated root directory after adding file1.txt:\n");
    print_directory(updated_root);
    free(updated_root);
    printf("\n");

    //we print free list head
    DiskInfo info;
    uint32_t num_fat_entries = disk_size / BLOCK_SIZE;
    uint32_t fat[num_fat_entries];
    read_info_and_fat(disk_memory, &info, fat, disk_size);
    print_disk_info(&info);
    printf("\n");

    //now we remove the directory we just created
    remove_directory(disk_memory, "dir1", cursor, disk_size);
    printf("\n");

    //we list the contents of root again
    printf("Contents of root directory after removing dir1:\n");
    list_directory_contents(disk_memory, cursor, disk_size);
    printf("\n");

    //now we remove the file we created
    remove_file(disk_memory, "file1.txt", cursor, disk_size);
    printf("\n");

    //we list the contents of root again
    printf("Contents of root directory after removing file1.txt:\n");
    list_directory_contents(disk_memory, cursor, disk_size);
    printf("\n");

    // Clean up
    close_and_unmap_disk(disk_memory, disk_size);

}
int main() {
    //test_function();
    shell_init();

    return 0;
}