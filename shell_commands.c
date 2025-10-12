#include "shell_commands.h"

//format
char* format_disk(const char *filename, size_t size) {
    bool DISK_EXISTS = false;     // flag to check if a disk is mounted
    char* disk_memory = NULL;         // Memory mapped disk
    //check if disk already exists
    FILE* file = fopen(filename, "r");
    if (file) {
        fclose(file);
        printf("Disk file already exists. Reading...\n");
        disk_memory = open_and_map_disk(filename, size);
        if (disk_memory == NULL) handle_error("Failed to open and map existing disk");
        DISK_EXISTS = true;
        //no need to write anything, just return the memory mapped disk
        return disk_memory;
    }
    // Create a new disk
    if(!DISK_EXISTS) {
        if (DEBUG) printf("Creating and formatting new disk...\n");
        disk_memory = open_and_map_disk(filename, size);
        if (disk_memory == NULL) handle_error("Failed to create and format new disk");
        //we need to initialize the disk info structure: metainfo, fat, root directory
        //Calculate reserved blocks
        uint32_t reserved_blocks = calc_reserved_blocks(size, BLOCK_SIZE);
        uint32_t metainfo_blocks = 1;
        uint32_t fat_blocks = reserved_blocks - metainfo_blocks;
        if(DEBUG) printf("Reserved blocks: %u (Metainfo: %u, FAT: %u)\n", reserved_blocks, metainfo_blocks, fat_blocks);
        // Initialize DiskInfo
        DiskInfo info = {0};
        info.block_size = BLOCK_SIZE;
        info.disk_size = size;
        info.free_blocks = (size / BLOCK_SIZE) - reserved_blocks;
        info.free_list_head = 0; // Allocate/append will set this correctly
        snprintf(info.name, MAX_NAME_LEN, "%s", filename);
        // Initialize FAT
        uint32_t num_blocks = size / BLOCK_SIZE;
        uint32_t num_fat_entries = num_blocks;
        uint32_t fat[num_fat_entries]; // Array to hold FAT entries
        init_fat(fat, num_fat_entries);
        if (DEBUG) printf("FAT initialized successfully.\n");
        //we allocate the metainfo block
        uint32_t result = allocate_block(fat, &info);
        if (result == FAT_EOF) handle_error("Failed to allocate block for metainfo");
        if (DEBUG) printf("Reserved block %d allocated for metainfo.\n", result);
        //we append the reserved blocks to the chain
        uint32_t index = 0;
        for (uint32_t i = 1; i < reserved_blocks; i++) {
            uint32_t new_block = append_block_to_chain(fat, &info, index);
            if (new_block == FAT_EOF) handle_error("Failed to append reserved block to chain");
            index++;
        }
        //write metainfo and fat to disk
        int res = write_metainfo(disk_memory, &info, BLOCK_SIZE, size);
        if (res != 0) handle_error("Failed to write metainfo to disk");
        if (DEBUG) printf("Metainfo written to disk successfully.\n");
        uint32_t fat_index = 1; // FAT starts at block 1
        res = write_fat(disk_memory, fat, num_fat_entries, fat_index, BLOCK_SIZE, size);
        if (res != 0) handle_error("Failed to write FAT to disk");
        if (DEBUG) printf("FAT written to disk successfully.\n");
        //add root directory
        //allocate block for root
        result = allocate_block(fat, &info);
        if (result == FAT_EOF) handle_error("Failed to allocate block for root directory");
        Entry root = {0};
        init_directory(&root, "/", result);
        root.current_block = result;
        root.parent_block = FAT_EOF;
        if (DEBUG) printf("Root directory initialized successfully.\n");
        //write root to disk
        res = write_entry(disk_memory, &root, BLOCK_SIZE, size);
        if (res != 0) handle_error("Failed to write root directory to disk");
        if (DEBUG) printf("Root directory written to disk successfully.\n");
        //update fat and metainfo
        res = write_info_and_fat(disk_memory, fat, num_fat_entries, 1, &info, BLOCK_SIZE, size);
        if (res != 0) handle_error("Failed to update FAT and metainfo after root directory creation");
        if (DEBUG) printf("FAT and metainfo updated successfully after root directory creation.\n");
    }
    return disk_memory;
}

//mkdir
void create_directory(char* disk_mem, const char *name, uint32_t parent_block, size_t disk_size_bytes) {
    //the new directory will be created inside the parent directory
    if (DEBUG) printf("Creating directory '%s' inside parent block %u\n", name, parent_block);
    //load metainfo and fat
    DiskInfo info;
    uint32_t num_fat_entries = disk_size_bytes / BLOCK_SIZE;
    uint32_t fat[num_fat_entries];
    read_info_and_fat(disk_mem, &info, fat, disk_size_bytes);
    if (DEBUG) {
        printf("In create_directory:\n");
        print_disk_info(&info);
        print_fat(fat, 10);
    }
    //check if there's space for a new directory
    if (info.free_blocks == 0) handle_error("No free blocks available to create new directory");
    //scan the children of the parent directory to check if a directory with the same name already exists
    //read parent directory
    Entry* parent_dir = read_directory_from_block(disk_mem, parent_block, BLOCK_SIZE, disk_size_bytes);
    if (parent_dir == NULL) handle_error("Failed to read parent directory");
    uint32_t* children_blocks = get_children_blocks(parent_dir);
    if (children_blocks == NULL) handle_error("Failed to get children blocks of parent directory");
    for (int i = 0; i < MAX_DIR_ENTRIES; i++) {
        if (children_blocks[i] == 0) continue; // No child in this slot
        Entry* child = read_directory_from_block(disk_mem, children_blocks[i], BLOCK_SIZE, disk_size_bytes);
        if (child == NULL) handle_error("Failed to read child directory");
        if (strcmp(child->name, name) == 0 && child->type == ENTRY_TYPE_DIR) {
            printf("Directory with the same name already exists in the parent directory");
            free(parent_dir);
            free(child);
            return;
        }
        free(child);
    }
    //if program reaches here, it's possible to create the new directory
    if (DEBUG) printf("Parent directory read successfully.\n");
    if (DEBUG) print_directory(parent_dir);
    //allocate block for new directory
    uint32_t new_dir_block = allocate_block(fat, &info);
    if (new_dir_block == FAT_EOF) handle_error("Failed to allocate block for new directory");
    //initialize new directory
    Entry new_dir;
    init_directory(&new_dir, name, new_dir_block);
    new_dir.current_block = new_dir_block;
    new_dir.parent_block = parent_block;
    //write new directory to disk
    int res = write_entry(disk_mem, &new_dir, BLOCK_SIZE, disk_size_bytes);
    if (res != 0) handle_error("Failed to write new directory to disk");
    if (DEBUG) printf("New directory written to disk successfully.\n");
    //add new directory to parent
    update_directory_children(parent_dir, new_dir_block);
    //write updated parent directory to disk
    res = write_entry(disk_mem, parent_dir, BLOCK_SIZE, disk_size_bytes);
    if (res != 0) handle_error("Failed to update parent directory on disk");
    if (DEBUG) printf("Parent directory updated successfully.\n");
    //update fat and metainfo on disk
    res = write_info_and_fat(disk_mem, fat, num_fat_entries, 1, &info, BLOCK_SIZE, disk_size_bytes);
    if (res != 0) handle_error("Failed to update FAT and metainfo after creating new directory");
    if (DEBUG) printf("FAT and metainfo updated successfully after creating new directory.\n");
    if (DEBUG) printf("Directory '%s' created successfully inside parent block %u\n", name, parent_block);
    if (DEBUG) {
        printf("Updated parent directory:\n");
        print_directory(parent_dir);
        printf("New directory:\n");
        print_directory(&new_dir);
    }
    if (DEBUG) {
        //print updated disk info and fat
        print_disk_info(&info);
        print_fat(fat, 10);
    }
    //clean up
    free(parent_dir);
}

//rmdir
void remove_directory(char* disk_mem, const char *name, uint32_t parent_block, size_t disk_size_bytes) {
    //load metainfo and fat
    DiskInfo info;
    uint32_t num_fat_entries = disk_size_bytes / BLOCK_SIZE;
    uint32_t fat[num_fat_entries];
    read_info_and_fat(disk_mem, &info, fat, disk_size_bytes);
    //read parent directory
    Entry* parent_dir = read_directory_from_block(disk_mem, parent_block, BLOCK_SIZE, disk_size_bytes);
    if (parent_dir == NULL) handle_error("Failed to read parent directory");
    //find the child directory to remove
    uint32_t* children_blocks = get_children_blocks(parent_dir);
    if (children_blocks == NULL) handle_error("Failed to get children blocks of parent directory");
    int dir_index = -1;
    Entry* dir_to_remove = NULL;
    for (int i = 0; i < MAX_DIR_ENTRIES; i++) {
        if (children_blocks[i] == 0) continue; // No child in this slot
        Entry* child = read_directory_from_block(disk_mem, children_blocks[i], BLOCK_SIZE, disk_size_bytes);
        if (child == NULL) handle_error("Failed to read child directory");
        if (strcmp(child->name, name) == 0 && child->type == ENTRY_TYPE_DIR) {
            dir_index = i;
            dir_to_remove = child;
            break;
        }
        free(child);
    }
    if (dir_index == -1 || dir_to_remove == NULL) {
        printf("Directory to remove not found in parent directory");
        free(parent_dir);
        return;
    }
    //check if directory is empty
    if (dir_to_remove->size > 0) {
        printf("Directory is not empty, cannot remove");
        free(parent_dir);
        free(dir_to_remove);
        return;
    }
    //deallocate directory block
    int res = deallocate_chain(fat, &info, dir_to_remove->current_block);
    if (res != 0) handle_error("Failed to deallocate directory block");
    //remove directory from parent by zeroing its entry
    parent_dir->dir_blocks[dir_index] = 0;
    parent_dir->size--;
    //write updated parent directory to disk
    res = write_entry(disk_mem, parent_dir, BLOCK_SIZE, disk_size_bytes);
    if (res != 0) handle_error("Failed to write updated parent directory to disk");
    //update fat and metainfo on disk
    res = write_info_and_fat(disk_mem, fat, num_fat_entries, 1, &info, BLOCK_SIZE, disk_size_bytes);
    if (res != 0) handle_error("Failed to update FAT and metainfo after removing directory");
    //clean up
    free(parent_dir);
}

//ls
void list_directory_contents(char* disk_mem, uint32_t cursor, size_t disk_size_bytes) {
    //read directory at cursor
    Entry* dir = read_directory_from_block(disk_mem, cursor, BLOCK_SIZE, disk_size_bytes);
    if (dir == NULL) handle_error("Failed to read directory at cursor");
    printf("Contents of directory '%s':\n", dir->name);
    uint32_t* children_blocks = get_children_blocks(dir);
    if (children_blocks == NULL) handle_error("Failed to get children blocks of directory");
    int has_children = 0;
    for (int i = 0; i < MAX_DIR_ENTRIES; i++) {
            if (children_blocks[i] == 0) continue; // No child in this slot
        Entry* child = read_directory_from_block(disk_mem, children_blocks[i], BLOCK_SIZE, disk_size_bytes);
        if (child == NULL) handle_error("Failed to read child entry");
        printf("- Name: %s - Type: %s - Size: %s\n", child->name, child->type == ENTRY_TYPE_DIR ? "Directory" : "File", format_size(child->size));
        free(child);
        has_children = 1;
    }
    if (!has_children) {
        printf("Directory is empty.\n");
    }
    free(dir);
}

//cd
uint32_t change_directory(const char *path, uint32_t cursor, char* disk_mem, size_t disk_size_bytes) {
    //this function returns a new cursor
    //it's possible to change from the parent directory to one of its children
    //or from a child directory to its parent (..)
    //load metainfo and fat
    uint32_t new_cursor = cursor;
    DiskInfo info;
    uint32_t num_fat_entries = disk_size_bytes / BLOCK_SIZE;
    uint32_t fat[num_fat_entries];
    read_info_and_fat(disk_mem, &info, fat, disk_size_bytes);
    //read current directory
    Entry* current_dir = read_directory_from_block(disk_mem, cursor, BLOCK_SIZE, disk_size_bytes);
    if (current_dir == NULL) handle_error("Failed to read current directory");
    //check if path is ".." to go to parent
    if (strcmp(path, "..") == 0) {
        if (current_dir->parent_block == FAT_EOF) {
            printf("Already at root directory, cannot go up.\n");
            free(current_dir);
            return cursor;
        }
        Entry* parent_dir = read_directory_from_block(disk_mem, current_dir->parent_block, BLOCK_SIZE, disk_size_bytes);
        if (parent_dir == NULL) handle_error("Failed to read parent directory");
        if (DEBUG) printf("Changed directory to parent: '%s'\n", parent_dir->name);
        new_cursor = parent_dir->current_block;
        free(current_dir);
        free(parent_dir);
        return new_cursor;
    }
    //look for the child directory with the given name
    uint32_t* children_blocks = get_children_blocks(current_dir);
    if (children_blocks == NULL) handle_error("Failed to get children blocks of current directory");
    int found = 0;
    for (int i = 0; i < MAX_DIR_ENTRIES; i++) {
        if (children_blocks[i] == 0) continue; // No child in this slot
        Entry* child = read_directory_from_block(disk_mem, children_blocks[i], BLOCK_SIZE, disk_size_bytes);
        if (child == NULL) handle_error("Failed to read child directory");
        if (strcmp(child->name, path) == 0 && child->type == ENTRY_TYPE_DIR) {
            if (DEBUG) printf("Changed directory to child: '%s'\n", child->name);
            new_cursor = child->current_block;
            found = 1;
            free(child);
            break;
        }
        free(child);
    }
    if (!found) {
        printf("Directory '%s' not found in current directory.\n", path);
    }
    free(current_dir);
    return new_cursor;
}

//touch
void create_file(char* disk_mem, const char* name, uint32_t parent_block, size_t disk_size_bytes){
    //allocate a new block with FAT (allocateBlock).
    DiskInfo info;
    uint32_t num_fat_entries = disk_size_bytes / BLOCK_SIZE;
    uint32_t fat[num_fat_entries];
    read_info_and_fat(disk_mem, &info, fat, disk_size_bytes);
    //check if there's space for a new file
    if (info.free_blocks == 0) handle_error("No free blocks available to create new file");
    //scan the children of the parent directory to check if a file with the same name already exists
    //read parent directory
    Entry* parent_dir = read_directory_from_block(disk_mem, parent_block, BLOCK_SIZE, disk_size_bytes);
    if (parent_dir == NULL) handle_error("Failed to read parent directory");
    if (DEBUG) printf("Parent directory read successfully.\n");
    uint32_t* children_blocks = get_children_blocks(parent_dir);
    if (children_blocks == NULL) handle_error("Failed to get children blocks of parent directory");
    for (int i = 0; i < MAX_DIR_ENTRIES; i++) {
        if (children_blocks[i] == 0) continue; // No child in this slot
        Entry* child = read_directory_from_block(disk_mem, children_blocks[i], BLOCK_SIZE, disk_size_bytes);
        if (child == NULL) handle_error("Failed to read child directory");
        if (strcmp(child->name, name) == 0 && child->type == ENTRY_TYPE_FILE) {
            printf("File with the same name already exists in the parent directory");
            free(child);
            free(parent_dir);
            return;
        }
        free(child);
    }
    //if program reaches here, it means it's possible to create the new file
    //allocate block for new file
    uint32_t new_file_block = allocate_block(fat, &info);
    if (new_file_block == FAT_EOF) handle_error("Failed to allocate block for new file");
    //initialize new file
    Entry new_file;
    init_file(&new_file, name, new_file_block);
    if (DEBUG) printf("shell_commands: Creating file with name: %s at block %u\n", new_file.name, new_file_block);
    new_file.name[MAX_NAME_LEN - 1] = '\0'; // Ensure null-termination
    new_file.current_block = new_file_block;
    new_file.parent_block = parent_block;
    //write new file to disk
    write_entry(disk_mem, &new_file, BLOCK_SIZE, disk_size_bytes);
    //update parent directory
    update_directory_children(parent_dir, new_file_block);
    //write updated parent directory to disk
    write_entry(disk_mem, parent_dir, BLOCK_SIZE, disk_size_bytes);
    //update fat and metainfo on disk
    int res = write_info_and_fat(disk_mem, fat, num_fat_entries, 1, &info, BLOCK_SIZE, disk_size_bytes);
    if (res != 0) handle_error("Failed to update FAT and metainfo after creating new file");
    if (DEBUG) printf("FAT and metainfo updated successfully after creating new file.\n");
    //clean up
    free(parent_dir);
}

//rm
void remove_file(char* disk_mem, const char *name, uint32_t parent_block, size_t disk_size_bytes){
    //load_info_and_fat
    DiskInfo info;
    uint32_t num_fat_entries = disk_size_bytes / BLOCK_SIZE;
    uint32_t fat[num_fat_entries];
    read_info_and_fat(disk_mem, &info, fat, disk_size_bytes);
    //read parent directory
    Entry* parent_dir = read_directory_from_block(disk_mem, parent_block, BLOCK_SIZE, disk_size_bytes);
    if (parent_dir == NULL) handle_error("Failed to read parent directory");
    //find file entry in parent (scan dir_blocks)
    uint32_t* children_blocks = get_children_blocks(parent_dir);
    if (children_blocks == NULL) handle_error("Failed to get children blocks of parent directory");
    int file_index = -1;
    Entry* file_entry = NULL;
    for (int i = 0; i < MAX_DIR_ENTRIES; i++) {
        if (children_blocks[i] == 0) continue; // No child in this slot
        Entry* child = read_directory_from_block(disk_mem, children_blocks[i], BLOCK_SIZE, disk_size_bytes);
        if (child == NULL) handle_error("Failed to read child entry");
        if (strcmp(child->name, name) == 0 && child->type == ENTRY_TYPE_FILE) {
            file_index = i;
            file_entry = child;
            break;
        }
        free(child);
    }
    if (file_index == -1 || file_entry == NULL) {
        printf("File to remove not found in parent directory");
        free(parent_dir);
        return;
    }
    //If program reaches here, it means it found the file to remove
    //compute total size to subtract from parent directory size
    uint32_t total_size = file_entry->size;
    int res = deallocate_chain(fat, &info, file_entry->current_block);
    if (res != 0) handle_error("Failed to deallocate file blocks");
    //update parent directory
    parent_dir->dir_blocks[file_index] = 0;
    parent_dir->size-= total_size;
    //write_entry (parent)
    res = write_entry(disk_mem, parent_dir, BLOCK_SIZE, disk_size_bytes);
    if (res != 0) handle_error("Failed to write updated parent directory to disk");
    //update_fat_and_metainfo
    res = write_info_and_fat(disk_mem, fat, num_fat_entries, 1, &info, BLOCK_SIZE, disk_size_bytes);
    if (res != 0) handle_error("Failed to update FAT and metainfo after removing file");
    //clean up
    free(parent_dir);
    free(file_entry);
}

//append
void append_to_file(char* disk_mem, char* data, size_t data_len, char* filename, uint32_t cursor, size_t block_size, size_t disk_size_bytes){
    //load info and FAT
    DiskInfo info;
    uint32_t num_fat_entries = disk_size_bytes / block_size;
    uint32_t fat[num_fat_entries];
    read_info_and_fat(disk_mem, &info, fat, disk_size_bytes);
    //find file_entry in current directory
    Entry* file_entry = NULL;
    Entry* current_dir = read_directory_from_block(disk_mem, cursor, block_size, disk_size_bytes);
    if (!current_dir) handle_error("Failed to read current directory");
    uint32_t* children_blocks = get_children_blocks(current_dir);
    if (!children_blocks) handle_error("Failed to get children blocks");
    for (int i = 0; i < MAX_DIR_ENTRIES; i++) {
        if (children_blocks[i] == 0) continue;
        Entry* child = read_directory_from_block(disk_mem, children_blocks[i], block_size, disk_size_bytes);
        if (!child) handle_error("Failed to read child directory");
        if (strcmp(child->name, filename) == 0 && child->type == ENTRY_TYPE_FILE) {
            file_entry = child;
            break;
        }
        free(child);
    }
    free(current_dir);
    if (!file_entry) {
        printf("File to append to not found in current directory\n");
        return;
    }
    uint32_t entry_block = file_entry->current_block;
    //find first data block, allocate one if it doesn't exist
    uint32_t data_block = fat[entry_block];
    if (data_block == FAT_EOC) {
        data_block = allocate_block(fat, &info);
        if (data_block == FAT_EOF) handle_error("No free blocks");
        fat[entry_block] = data_block;
        fat[data_block] = FAT_EOC;
    }
    //scroll the chain until the last data block
    uint32_t last_data_block = data_block;
    while (fat[last_data_block] != FAT_EOC) last_data_block = fat[last_data_block];
    //compute write offset in block
    size_t offset = file_entry->size % block_size;
    size_t to_write = data_len;
    size_t written = 0;
    while (to_write > 0) {
        char buffer[block_size];
        //if offset > 0, read the block to avoid overwriting existing data
        if (offset > 0) {
            int res = read_block(disk_mem, last_data_block, buffer, block_size, disk_size_bytes);
            if (res != 0) handle_error("Failed to read last data block for append");
        } else {
            memset(buffer, 0, block_size);
        }
        size_t space = block_size - offset;
        size_t chunk = (to_write < space) ? to_write : space;
        memcpy(buffer + offset, data + written, chunk);
        int res = write_block(disk_mem, last_data_block, buffer, block_size, disk_size_bytes);
        if (res != 0) handle_error("Failed to write appended block to disk");
        file_entry->size += chunk;
        written += chunk;
        to_write -= chunk;
        offset = 0;
        //if there is still data, allocate a new block and continue
        if (to_write > 0) {
            uint32_t new_block = allocate_block(fat, &info);
            if (new_block == FAT_EOF) handle_error("No free blocks for additional data block");
            fat[last_data_block] = new_block;
            fat[new_block] = FAT_EOC;
            last_data_block = new_block;
        }
    }
    int res = write_entry(disk_mem, file_entry, block_size, disk_size_bytes);
    if (res != 0) handle_error("Failed to write updated file entry to disk");
    //update parent directory size
    Entry* parent_dir = read_directory_from_block(disk_mem, file_entry->parent_block, block_size, disk_size_bytes);
    if (!parent_dir) handle_error("Failed to read parent directory");
    parent_dir->size += data_len;
    res = write_entry(disk_mem, parent_dir, block_size, disk_size_bytes);
    if (res != 0) handle_error("Failed to update parent directory size");
    free(parent_dir);
    res = write_info_and_fat(disk_mem, fat, num_fat_entries, 1, &info, block_size, disk_size_bytes);
    if (res != 0) handle_error("Failed to update FAT/metainfo after append");
    free(file_entry);
}

// cat
void cat_file(char* disk_mem, const char* filename, uint32_t cursor, size_t block_size, size_t disk_size_bytes){
    DiskInfo info;
    uint32_t num_fat_entries = disk_size_bytes / block_size;
    uint32_t fat[num_fat_entries];
    read_info_and_fat(disk_mem, &info, fat, disk_size_bytes);
    Entry* file_entry = NULL;
    Entry* current_dir = read_directory_from_block(disk_mem, cursor, block_size, disk_size_bytes);
    if (!current_dir) handle_error("Failed to read current directory");
    uint32_t* children_blocks = get_children_blocks(current_dir);
    if (!children_blocks) handle_error("Failed to get children blocks");
    for (int i = 0; i < MAX_DIR_ENTRIES; i++) {
        if (children_blocks[i] == 0) continue;
        Entry* child = read_directory_from_block(disk_mem, children_blocks[i], block_size, disk_size_bytes);
        if (!child) handle_error("Failed to read child directory");
        if (strcmp(child->name, filename) == 0 && child->type == ENTRY_TYPE_FILE) {
            file_entry = child;
            break;
        }
        free(child);
    }
    free(current_dir);
    if (!file_entry) {
        printf("File to read not found in current directory\n");
        return;
    }
    uint32_t data_block = fat[file_entry->current_block];
    size_t bytes_left = file_entry->size;
    while (data_block != FAT_EOC && bytes_left > 0) {
        char buffer[block_size];
        memset(buffer, 0, block_size);
        int res = read_block(disk_mem, data_block, buffer, block_size, disk_size_bytes);
        if (res != 0) handle_error("Failed to read data block");
        size_t to_print = (bytes_left < block_size) ? bytes_left : block_size;
        fwrite(buffer, 1, to_print, stdout);
        bytes_left -= to_print;
        data_block = fat[data_block];
    }
    printf("\n");
    free(file_entry);
}