#include "disk.h"
#include "shell.h"
#include "fat.h"
#include "entry.h"
#include "shell_commands.h"

void test_function() {
    // Init disk
    const char* disk_path = "disk.img";
    size_t disk_size = 16 * 1024 * 1024; // 16 MB

    // Format disk
    char* disk_memory = format_disk(disk_path, disk_size);
    if (disk_memory == NULL) handle_error("Failed to initialize disk");

    printf("Disk initialized successfully.\n\n");
    print_disk_status(disk_memory, disk_size);
    printf("\n");

    // Set root block and cursor
    uint32_t reserved_blocks = calc_reserved_blocks(disk_size, BLOCK_SIZE);
    uint32_t root_block = reserved_blocks;
    Entry* read_root = read_directory_from_block(disk_memory, root_block, BLOCK_SIZE, disk_size);
    if (read_root == NULL) handle_error("Failed to read root directory");
    printf("Root directory:\n");
    print_directory(read_root);
    free(read_root);

    uint32_t cursor = root_block;
    printf("Cursor at root block: %u\n\n", cursor);

    // Create file
    create_file(disk_memory, "file1.txt", cursor, disk_size);
    printf("\n");

    // List directory contents
    printf("Contents of root directory after creating file1.txt:\n");
    list_directory_contents(disk_memory, cursor, disk_size);
    printf("\n");

    //append to file
    char* data = "Hello, this is some appended data to file1.txt!";
    size_t data_len = strlen(data);
    append_to_file(disk_memory, data, data_len, "file1.txt", cursor, BLOCK_SIZE, disk_size);
    printf("\n");

    //read file with cat command
    printf("Contents of file1.txt after appending data:\n");
    cat_file(disk_memory, "file1.txt", cursor, BLOCK_SIZE, disk_size);
    printf("\n");

    /*
    
    //add a new directory
    create_directory(disk_memory, "dir1", cursor, disk_size);
    printf("\n");
    
    // List directory contents
    printf("Contents of root directory after creating dir1:\n");
    list_directory_contents(disk_memory, cursor, disk_size);
    printf("\n");
    */

    //print fat
    DiskInfo info;
    uint32_t num_fat_entries = disk_size / BLOCK_SIZE;
    uint32_t fat[num_fat_entries];
    read_info_and_fat(disk_memory, &info, fat, disk_size);
    printf("FAT entries after operations:\n");
    print_fat(fat, 10);
    printf("\n");

    // Clean up
    close_and_unmap_disk(disk_memory, disk_size);
}

int main() {
    //test_function();
    shell_init();

    return 0;
}