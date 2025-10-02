#include "disk.h"
#include "shell.h"
#include "fat.h"
#include "entry.h"

#define MAX_TOKENS 3
#define MAX_COMMAND_LENGTH 128

void shell_init() {
    // Persistent disk info and structures
    char* disk_memory = NULL;         // Memory mapped disk
    DiskInfo info = {0};              // Disk metadata
    uint32_t* fat = NULL;             // Pointer to FAT
    uint32_t num_fat_entries = 0;     // Number of FAT entries
    size_t disk_size = 0;             // Disk size
    uint32_t block_size = 0;          // Block size

    // Cursor for current directory: contains the block index of the current directory
    uint32_t cursor = 0;
    char current_path[128] = "/";     // initial path (root)
    bool DISK_IS_MOUNTED = false;     // flag to check if a disk is mounted

    printf("\nWelcome to FS Shell!\n");

    while (1) {
        printf("\ntype 'help' for a list of commands\n");
        printf("----------------------\n");
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
        if (strncmp(comm, "help", 4) == 0) {
            printf("Available commands:\n");
            printf(" - format <fs_filename> <size>\n");
            printf(" - mkdir <dir_name>\n");
            printf(" - cd <dir_name>\n");
            printf(" - touch <file_name>\n");
            printf(" - cat <file_name>\n");
            printf(" - ls\n");
            printf(" - append <file> <text>\n");
            printf(" - rm <dir/file>\n");
            printf(" - close\n");
            continue;
        }

        //close command
        else if (strncmp(comm, "close", 5) == 0) {
            printf("Exiting shell...\n");
            break;
        }

        //format command
        else if (strncmp(comm, "format", 6) == 0) {
            // Check if name and size are provided
            if (tokens[1] == NULL || tokens[2] == NULL) {
                printf("Error: missing arguments\n");
                continue;
            }
            char* fs_filename = tokens[1];
            int size = atoi(tokens[2]);
            if (strlen(fs_filename) >= MAX_NAME_LEN) {
                printf("Error: filename too long. Maximum length is %d\n", MAX_NAME_LEN - 1);
                continue;
            }
            if (size != 16 && size != 32 && size != 64) {
                printf("Error: invalid size. Allowed sizes are 16, 32, 64\n");
                continue;
            }
            printf("Formatting disk...\n");

            // Free previous FAT if any
            if (fat != NULL) {
                free(fat);
                fat = NULL;
            }
            // Unmap previous disk if any
            if (disk_memory != NULL) {
                close_and_unmap_disk(disk_memory, disk_size);
                disk_memory = NULL;
            }

            disk_size = size * 1024 * 1024; // size in MB
            block_size = 4096; // 4 KB
            disk_memory = open_and_map_disk(fs_filename, disk_size);
            if (!disk_memory) {
                perror("Failed to initialize disk");
                continue;
            }
            printf("Disk formatted successfully\n");

            uint32_t reserved_blocks = calc_reserved_blocks(disk_size, block_size);
            printf("Reserved blocks: %u\n", reserved_blocks);

            info.block_size = block_size;
            info.disk_size = disk_size;
            info.free_blocks = (disk_size / block_size);
            info.free_list_head = 0; // First free block after reserved blocks
            snprintf(info.name, MAX_NAME_LEN, "%s", fs_filename);

            num_fat_entries = disk_size / block_size;
            fat = malloc(num_fat_entries * sizeof(uint32_t));
            if (!fat) {
                printf("FAT malloc failed\n");
                continue;
            }
            init_fat(fat, num_fat_entries);
            printf("FAT initialized successfully.\n");

            // Allocate metainfo block
            uint32_t res = allocateBlock(fat, &info);
            if (res == FAT_EOF) {
                handle_error("Failed to allocate block for metainfo");
            }
            printf("Reserved block allocated for metainfo.\n");

            // Append reserved blocks to chain
            uint32_t index = 0;
            for (uint32_t i = 1; i < reserved_blocks; i++) {
                int ret = appendBlockToChain(fat, &info, index);
                if (ret == -1) {
                    handle_error("Failed to append reserved block to chain");
                }
                index++;
            }

            res = write_metainfo(disk_memory, &info, block_size, disk_size);
            if (res != 0) {
                handle_error("Failed to write metainfo to disk");
            }
            printf("Metainfo written to disk successfully.\n");
            res = write_fat(disk_memory, fat, num_fat_entries, 1, block_size, disk_size);
            if (res != 0) {
                handle_error("Failed to write FAT to disk");
            }
            printf("FAT written to disk successfully.\n");

            // Add root directory
            Entry root = {0};
            snprintf(root.name, MAX_NAME_LEN, "/");
            root.type = ENTRY_TYPE_DIR;
            root.size = 1;
            root.current_block = info.free_list_head; // first free block
            root.next_block = FAT_EOC;

            uint32_t root_block = allocateBlock(fat, &info);
            if (root_block == FAT_EOF) {
                handle_error("Failed to allocate block for root directory");
            }
            root.current_block = root_block;
            printf("Root directory allocated at block %d\n", root_block);

            char root_buffer[block_size];
            memset(root_buffer, 0, block_size);
            memcpy(root_buffer, &root, sizeof(Entry));
            res = write_block(disk_memory, root.current_block, root_buffer, block_size, disk_size);
            if (res != 0) {
                handle_error("Failed to write root directory to disk");
            }
            printf("Root directory written to disk successfully.\n");

            res = write_metainfo(disk_memory, &info, block_size, disk_size);
            if (res != 0) {
                handle_error("Failed to update metainfo on disk after root creation");
            }
            printf("Metainfo updated on disk successfully after root creation.\n");

            res = write_fat(disk_memory, fat, num_fat_entries, 1, block_size, disk_size);
            if (res != 0) {
                handle_error("Failed to update FAT on disk after root creation");
            }
            printf("FAT updated on disk successfully after root creation.\n");

            cursor = root.current_block; //set cursor to root
            snprintf(current_path, sizeof(current_path), "/");
            printf("Current directory set to root.\n");

            DISK_IS_MOUNTED = true;
            continue;
        }

        //mkdir command
        else if (strncmp(comm, "mkdir", 5) == 0) {
            if (!DISK_IS_MOUNTED) {
                printf("Error: no disk mounted. Please format a disk first.\n");
                continue;
            }
            if (tokens[1] == NULL) {
                printf("Error: missing arguments\n");
                continue;
            }
            char* dir_name = tokens[1];

            //check if dir_name is already present in current directory
            
            //we need to scan the entry array of the current directory
            Entry current_dir = {0};
            char curr_dir_buffer[block_size];
            memset(curr_dir_buffer, 0, block_size);
            int result = read_block(disk_memory, cursor, curr_dir_buffer, block_size, disk_size);
            if (result != 0) {
                handle_error("Failed to read current directory from disk");
            }
            memcpy(&current_dir, curr_dir_buffer, sizeof(Entry));
            for(uint32_t i = 0; i < current_dir.size; i++) {
                uint32_t entry_block = current_dir.dir_blocks[i];
                Entry entry = {0};
                char entry_buffer[block_size];
                memset(entry_buffer, 0, block_size);
                result = read_block(disk_memory, entry_block, entry_buffer, block_size, disk_size);
                if (result != 0) {
                    handle_error("Failed to read directory entry from disk");
                }
                memcpy(&entry, entry_buffer, sizeof(Entry));
                if (strncmp(entry.name, dir_name, MAX_NAME_LEN) == 0) {
                    printf("Error: directory or file with name '%s' already exists in current directory\n", dir_name);
                    break;
                }
            }

            printf("Creating directory...\n");
            Entry new_dir = {0};
            snprintf(new_dir.name, MAX_NAME_LEN, "%s", dir_name);
            new_dir.type = ENTRY_TYPE_DIR;
            new_dir.size = 1;

            uint32_t dir_block = allocateBlock(fat, &info);
            if (dir_block == FAT_EOF) {
                handle_error("Failed to allocate block for new directory");
            }
            new_dir.current_block = dir_block;
            new_dir.next_block = FAT_EOC;

            char dir_buffer[block_size];
            memset(dir_buffer, 0, block_size);
            memcpy(dir_buffer, &new_dir, sizeof(Entry));
            int res = write_block(disk_memory, new_dir.current_block, dir_buffer, block_size, disk_size);
            if (res != 0) {
                handle_error("Failed to write new directory to disk");
            }
            printf("New directory written to disk successfully.\n");

            Entry read_dir = {0};
            char read_buffer[block_size];
            memset(read_buffer, 0, block_size);
            res = read_block(disk_memory, new_dir.current_block, read_buffer, block_size, disk_size);
            if (res != 0) {
                handle_error("Failed to read back new directory from disk");
            }
            memcpy(&read_dir, read_buffer, sizeof(Entry));
            printf("Read back directory: Name: %s, Type: %s, Size: %u, Current Block: %u\n", 
                read_dir.name, 
                read_dir.type == ENTRY_TYPE_DIR ? "Directory" : "File",
                read_dir.size,
                read_dir.current_block);

            res = write_fat(disk_memory, fat, num_fat_entries, 1, block_size, disk_size);
            if (res != 0) {
                handle_error("Failed to update FAT on disk after directory creation");
            }
            printf("FAT updated on disk successfully after directory creation.\n");

            res = write_metainfo(disk_memory, &info, block_size, disk_size);
            if (res != 0) {
                handle_error("Failed to update metainfo on disk after directory creation");
            }
            printf("Metainfo updated on disk successfully after directory creation.\n");

            uint32_t* read_fat_array = malloc(num_fat_entries * sizeof(uint32_t));
            if (read_fat_array) {
                res = read_fat(disk_memory, read_fat_array, num_fat_entries, 1, block_size, disk_size);
                if (res == 0) {
                    printf("FAT read from disk successfully.\n");
                    print_fat(read_fat_array, 10);
                }
                free(read_fat_array);
            }
            printf("Directory created successfully: %s\n", dir_name);
            continue;
        }

        //cd command
        else if (strncmp(comm, "cd", 2) == 0) {
            if (!DISK_IS_MOUNTED) {
                printf("Error: no disk mounted. Please format a disk first.\n");
                continue;
            }
            printf("Changing directory...\n");
            // TODO: Implement directory change logic and update cursor/current_path
            continue;
        }

        //touch command
        else if (strncmp(comm, "touch", 5) == 0) {
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
            printf("Created empty file: %s\n", file_name);
            // TODO: Implement file creation
            continue;
        }

        //append command
        else if (strncmp(comm, "append", 6) == 0) {
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
        else if (strncmp(comm, "rm", 2) == 0) {
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
            // TODO: Implement remove logic
            continue;
        }

        //ls command
        else if (strncmp(comm, "ls", 2) == 0) {
            if (!DISK_IS_MOUNTED) {
                printf("Error: no disk mounted. Please format a disk first.\n");
                continue;
            }
            printf("Listing files...\n");
            // TODO: Implement list logic
            continue;
        }

        //cat command
        else if (strncmp(comm, "cat", 3) == 0) {
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

    // Cleanup on exit
    if (disk_memory != NULL) {
        close_and_unmap_disk(disk_memory, disk_size);
        disk_memory = NULL;
    }
    if (fat != NULL) {
        free(fat);
        fat = NULL;
    }
    printf("shell closed\n");
}