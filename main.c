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
    printf("\n");
    print_disk_status(disk_memory, disk_size);
    printf("\n");

    //we know root is after metainfo and fat reserved blocks
    uint32_t reserved_blocks = calc_reserved_blocks(disk_size, BLOCK_SIZE);
    uint32_t root_block = reserved_blocks; // Assuming root is the first block after reserved
    //uint32_t cursor = root_block;
    //read root directory
    Entry* read_root = read_directory_from_block(disk_memory, root_block, BLOCK_SIZE, disk_size);
    if (read_root == NULL) handle_error("Failed to read root directory");
    
    //printf("Root directory read successfully.\n");    
    printf("Root directory:\n");
    print_directory(read_root);

    free(read_root);

    uint32_t cursor = root_block;
    printf("Cursor at root block: %u\n", cursor);
    printf("\n");

    //create a new directory in root
    create_directory(disk_memory, "dir1", cursor, disk_size);
    //we point cursor to dir1
    //we read root to find dir1's block
    Entry* root_after_dir1 = read_directory_from_block(disk_memory, root_block, BLOCK_SIZE, disk_size);
    if (root_after_dir1 == NULL) handle_error("Failed to read root directory after creating dir1");
    uint32_t* children_blocks = get_children_blocks(root_after_dir1);
    if (children_blocks == NULL) handle_error("Failed to get children blocks of root directory");
    uint32_t new_cursor = cursor;
    for (int i = 0; i < MAX_DIR_ENTRIES; i++) {
        if (children_blocks[i] == 0) break; // No more children
        Entry* child = read_directory_from_block(disk_memory, children_blocks[i], BLOCK_SIZE, disk_size);
        if (child == NULL) handle_error("Failed to read child directory");
        if (strcmp(child->name, "dir1") == 0 && child->type == ENTRY_TYPE_DIR) {
            new_cursor = child->current_block;
            printf("New cursor moved to 'dir1' block: %u\n", new_cursor);
            free(child);
            break;
        }
        free(child);
    }
    Entry* dir1 = read_directory_from_block(disk_memory, new_cursor, BLOCK_SIZE, disk_size);
    if (dir1 == NULL) handle_error("Failed to read directory 'dir1'");
    printf("Directory 'dir1': \n");
    print_directory(dir1);
    free(dir1);

    //we read root again to see if dir1 is there
    printf("Reading the root again...\n");
    Entry* read_root_again = read_directory_from_block(disk_memory, root_block, BLOCK_SIZE, disk_size);
    if (read_root_again == NULL) handle_error("Failed to read root directory");
    print_directory(read_root_again);
    free(read_root_again);
    //fat and metainfo should be updated already by create_directory. We can print them again
    
    //we read status again
    print_disk_status(disk_memory, disk_size);

    //we now print with list_directory_contents
    printf("\n");
    list_directory_contents(disk_memory, cursor, disk_size);
    printf("\n");
    //function should have made side effect on cursor
    //we change directory to dir1
    printf("Cursor: %u\n", cursor);
    uint32_t another_new_cursor = change_directory("dir1", cursor, disk_memory, disk_size);
    printf("Cursor: %u\n", another_new_cursor);

    //we create a file in dir1
    create_file(disk_memory, "file1.txt", another_new_cursor, disk_size);
    //we list dir1 contents
    printf("\n");
    list_directory_contents(disk_memory, another_new_cursor, disk_size);
    printf("\n");

    //we remove the file
    remove_file(disk_memory, "file1.txt", another_new_cursor, disk_size);
    //we list dir1 contents again
    printf("\n");
    list_directory_contents(disk_memory, another_new_cursor, disk_size);
    printf("\n");

    /*
    //now we move to dir1 from root
    //now we read the directory at cursor (should be root)
    Entry* dir_at_cursor = read_directory_from_block(disk_memory, cursor, BLOCK_SIZE, disk_size);
    if (dir_at_cursor == NULL) handle_error("Failed to read directory at cursor");
    printf("Directory at cursor after changing to parent:\n");
    print_directory(dir_at_cursor);
    free(dir_at_cursor);
    */

    // Clean up
    close_and_unmap_disk(disk_memory, disk_size);

}
int main() {
    test_function();
    //shell_init();

    return 0;
}