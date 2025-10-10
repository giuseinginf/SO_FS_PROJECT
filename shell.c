#include "disk.h"
#include "shell.h"
#include "fat.h"
#include "entry.h"
#include "shell_commands.h"

#define MAX_TOKENS 3
#define MAX_COMMAND_LENGTH 128

void shell_init() {
    // Persistent disk info and structures
    char* disk_memory = NULL;         // Memory mapped disk
    uint32_t* fat = NULL;             // Pointer to FAT
    size_t disk_size = 0;             // Disk size

    uint32_t reserved_blocks = 0; // Number of reserved blocks (metainfo + FAT)
    uint32_t root_block = 0;      // Block index of the root directory

    // Cursor for current directory: contains the block index of the current directory
    uint32_t cursor = 0;
    char current_path[128] = "/";     // initial path (root)
    bool DISK_IS_MOUNTED = false;     // flag to check if a disk is mounted

    printf("\nWelcome to FS Shell!\n");

    while (1) {
        printf("\ntype 'help' for a list of commands\n");
        printf("----------------------\n");
        get_current_path(disk_memory, cursor, BLOCK_SIZE, disk_size, current_path, sizeof(current_path));
        printf("SHELL:%s$ ", current_path);
        char command[MAX_COMMAND_LENGTH];
        if (!fgets(command, MAX_COMMAND_LENGTH, stdin)) {
            printf("Error reading input\n");
            continue;
        }

        //remove newline character
        command[strcspn(command, "\n")] = 0;
        if (strlen(command) == 0) {
            printf("Invalid input: no command entered\n");
            continue;
        }

        //tokenize command
        char* tokens[MAX_TOKENS + 1];
        int i = 0;
        char* token = strtok(command, " \t");
        while (token != NULL && i < MAX_TOKENS) {
            tokens[i++] = token;
            token = strtok(NULL, " \t");
        }
        if (token != NULL) {
            printf("Error: too many arguments. Maximum is %d\n", MAX_TOKENS);
            continue;
        }
        tokens[i] = NULL;  // terminator
        char* comm = tokens[0];

        //help command
        if (strcmp(comm, "help") == 0) {
            printf("Available commands:\n");
            printf(" - format <fs_filename> <size>: create or open disk\n");
            printf(" - mkdir <dir_name>: create new directory\n");
            printf(" - cd <dir_name>: change directory\n");
            printf(" - touch <file_name>: create new file\n");
            printf(" - cat <file_name>: display file contents\n");
            printf(" - ls: list directory contents\n");
            printf(" - append <file_name> <text>: append text to file\n");
            printf(" - rm <file_name>: remove file\n");
            printf(" - rmdir <dir_name>: remove directory\n");
            printf(" - close\n");
            continue;
        }

        //close command
        else if (strcmp(comm, "close") == 0) {
            if (!DISK_IS_MOUNTED) {
                printf("No disk is currently mounted.\n");
                continue;
            }
            printf("Exiting shell...\n");
            close_and_unmap_disk(disk_memory, disk_size);
            break;
        }

        //format command
        else if (strcmp(comm, "format") == 0) {
            //we expect 2 arguments: filename and size
            if (tokens[1] == NULL || tokens[2] == NULL) {
                printf("Error: missing arguments\n");
                continue;
            }
            char* filename = tokens[1];
            char* size_str = tokens[2];
            size_t size = strtoul(size_str, NULL, 10);
            if (size != 16 && size != 32 && size != 64) {
                printf("Error: invalid size\n");
                continue;
            }
            disk_size = size * 1024 * 1024; // convert to bytes
            if (fat != NULL) {
                free(fat);
                fat = NULL;
            }
            disk_memory = format_disk(filename, disk_size);
            if (disk_memory == NULL) handle_error("Failed to format disk");
            printf("\n");
            print_disk_status(disk_memory, disk_size);
            printf("\n");

            reserved_blocks = calc_reserved_blocks(disk_size, BLOCK_SIZE);
            root_block = reserved_blocks; // Assuming root is the first block after reserved

            Entry* root = read_directory_from_block(disk_memory, root_block, BLOCK_SIZE, disk_size);
            if (root == NULL) handle_error("Failed to read root directory");
            //printf("Root directory:\n");
            //print_directory(root);
            free(root);

            cursor = root_block;
            strncpy(current_path, "/", sizeof(current_path));
            //printf("Cursor at root block: %u\n", cursor);
            DISK_IS_MOUNTED = true;

            continue;

        }

        //mkdir command
        else if (strcmp(comm, "mkdir") == 0) {
            if (!DISK_IS_MOUNTED) {
                printf("Error: no disk mounted. Please format a disk first.\n");
                continue;
            }
            if (tokens[1] == NULL) {
                printf("Error: missing arguments\n");
                continue;
            }
            char* dir_name = tokens[1];
            
            //printf("Creating directory: %s\n", dir_name);
            
            //cursor is the block of the current directory, already set in cd or at startup
            //we create the new directory inside the current directory
            create_directory(disk_memory, dir_name, cursor, disk_size);
            printf("Directory created: %s\n", dir_name);

            //we can print the updated current directory
            Entry* current_dir = read_directory_from_block(disk_memory, cursor, BLOCK_SIZE, disk_size);
            if (current_dir == NULL) handle_error("Failed to read current directory");
            
            /*
            printf("Updated current directory:\n");
            print_directory(current_dir);
            */
            
            free(current_dir);
            continue;
        }

        //cd command
        else if (strcmp(comm, "cd") == 0) {
            if (!DISK_IS_MOUNTED) {
                printf("Error: no disk mounted. Please format a disk first.\n");
                continue;
            }
            printf("Changing directory...\n");
            //cursor has to be updated to the new directory block
            if (tokens[1] == NULL) {
                printf("Error: missing arguments\n");
                continue;
            }
            char* dir_name = tokens[1];
            cursor = change_directory(dir_name, cursor, disk_memory, disk_size);
            if (cursor == FAT_EOF) {
                printf("Error: failed to change directory\n");
                continue;
            }
            printf("Changed directory to: %s\n", dir_name);
            continue;
        }        
        
        //rmdir command
        else if (strcmp(comm, "rmdir") == 0) {
            if (!DISK_IS_MOUNTED) {
                printf("Error: no disk mounted. Please format a disk first.\n");
                continue;
            }
            if (tokens[1] == NULL) {
                printf("Error: missing arguments\n");
                continue;
            }
            char* dir_name = tokens[1];
            printf("Removing directory: %s\n", dir_name);
            remove_directory(disk_memory, dir_name, cursor, disk_size);
            printf("Directory removed: %s\n", dir_name);
            //we can print the updated current directory
            Entry* current_dir = read_directory_from_block(disk_memory, cursor, BLOCK_SIZE, disk_size);
            if (current_dir == NULL) handle_error("Failed to read current directory");
            printf("Updated current directory:\n");
            print_directory(current_dir);
            free(current_dir);
            continue;
        }
        
        //ls command
        else if (strcmp(comm, "ls") == 0) {
            if (!DISK_IS_MOUNTED) {
                printf("Error: no disk mounted. Please format a disk first.\n");
                continue;
            }
            printf("Listing files...\n");
            //we now list the contents of the current directory
            list_directory_contents(disk_memory, cursor, disk_size);
            continue;
        }
        
        //touch command
        else if (strcmp(comm, "touch") == 0) {
            if (!DISK_IS_MOUNTED) {
                printf("Error: no disk mounted. Please format a disk first.\n");
                continue;
            }
            if (tokens[1] == NULL) {
                printf("Error: missing arguments\n");
                continue;
            }
            char* file_name = tokens[1];
            printf("Creating empty file...\n");
            create_file(disk_memory, file_name, cursor, disk_size);
            printf("Created empty file: %s\n", file_name);
            continue;
        }

        //append command
        else if (strcmp(comm, "append") == 0) {
            if (!DISK_IS_MOUNTED) {
                printf("Error: no disk mounted. Please format a disk first.\n");
                continue;
            }
            if (tokens[1] == NULL || tokens[2] == NULL) {
                printf("Error: missing arguments\n");
                continue;
            }
            char* file_name = tokens[1];
            char* text = tokens[2];
            printf("Appending to file: %s\n", file_name);
            printf("Text to append: %s\n", text);
            // TODO: Implement append logic
            continue;
        }

        //rm command
        else if (strcmp(comm, "rm") == 0) {
            if (!DISK_IS_MOUNTED) {
                printf("Error: no disk mounted. Please format a disk first.\n");
                continue;
            }
            if (tokens[1] == NULL) {
                printf("Error: missing arguments\n");
                continue;
            }
            char* file_name = tokens[1];
            printf("Removing file: %s\n", file_name);
            remove_file(disk_memory, file_name, cursor, disk_size);
            printf("File removed: %s\n", file_name);
            continue;
        }
        

        //cat command
        else if (strcmp(comm, "cat") == 0) {
            if (!DISK_IS_MOUNTED) {
                printf("Error: no disk mounted. Please format a disk first.\n");
                continue;
            }
            if (tokens[1] == NULL) {
                printf("Error: missing arguments\n");
                continue;
            }
            char* file_name = tokens[1];
            printf("Displaying content of file: %s\n", file_name);
            // TODO: Implement cat logic
            continue;
        }

        else {
            printf("Unknown command: %s\n", comm);
            continue;
        }
    }

    // Cleanup on exit. we do not unmap if already done in close command
    if (fat != NULL) {
        free(fat);
        fat = NULL;
    }
    printf("shell closed\n");
}