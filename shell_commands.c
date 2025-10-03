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

        printf("Creating and formatting new disk...\n");
        disk_memory = open_and_map_disk(filename, size);
        if (disk_memory == NULL) handle_error("Failed to create and format new disk");
        
        //we need to initialize the disk info structure: metainfo, fat, root directory
        //Calculate reserved blocks
        uint32_t reserved_blocks = calc_reserved_blocks(size, BLOCK_SIZE);
        uint32_t metainfo_blocks = 1;
        uint32_t fat_blocks = reserved_blocks - metainfo_blocks;
        printf("Reserved blocks: %u (Metainfo: %u, FAT: %u)\n", reserved_blocks, metainfo_blocks, fat_blocks);
        
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
        printf("FAT initialized successfully.\n");
        
        //we allocate the metainfo block
        uint32_t result = allocateBlock(fat, &info);
        if (result == FAT_EOF) handle_error("Failed to allocate block for metainfo");
        printf("Reserved block %d allocated for metainfo.\n", result);
        
        //we append the reserved blocks to the chain
        uint32_t index = 0;
        for (uint32_t i = 1; i < reserved_blocks; i++) {
            uint32_t new_block = appendBlockToChain(fat, &info, index);
            if (new_block == FAT_EOF) handle_error("Failed to append reserved block to chain");
            index++;
        }
        
        //write metainfo and fat to disk
        int res = write_metainfo(disk_memory, &info, BLOCK_SIZE, size);
        if (res != 0) handle_error("Failed to write metainfo to disk");
        printf("Metainfo written to disk successfully.\n");
        res = write_fat(disk_memory, fat, num_fat_entries, 1, BLOCK_SIZE, size);
        if (res != 0) handle_error("Failed to write FAT to disk");
        printf("FAT written to disk successfully.\n");
        
        //add root directory
        //allocate block for root
        result = allocateBlock(fat, &info);
        if (result == FAT_EOF) handle_error("Failed to allocate block for root directory");
        Entry root = {0};
        init_directory(&root, "/", result, ENTRY_TYPE_DIR);
        root.current_block = result;
        printf("Root directory initialized successfully.\n");
        
        //write root to disk
        res = write_directory(disk_memory, &root, BLOCK_SIZE, size);
        if (res != 0) handle_error("Failed to write root directory to disk");
        printf("Root directory written to disk successfully.\n");
        
        //update fat and metainfo
        res = update_fat_and_metainfo(disk_memory, fat, num_fat_entries, 1, &info, BLOCK_SIZE, size);
        if (res != 0) handle_error("Failed to update FAT and metainfo after root directory creation");
        printf("FAT and metainfo updated successfully after root directory creation.\n");
    }
    return disk_memory;
}

//mkdir
void create_directory(char* disk_mem, const char *name, uint32_t parent_block, size_t disk_size_bytes) {
    //the new directory will be created inside the parent directory
    
    //printf("Creating directory '%s' inside parent block %u\n", name, parent_block);
    
    //we need to load metainfo and fat first
    DiskInfo info;
    uint32_t num_fat_entries = disk_size_bytes / BLOCK_SIZE;
    uint32_t fat[num_fat_entries];
    load_info_and_fat(disk_mem, &info, fat, disk_size_bytes);
    
    //printf("In create_directory:\n");
    //print_disk_info(&info);
    //print_fat(fat, 10);
    
    //now that we have metainfo and fat in RAM, we can proceed
    //first we check if there's space for a new directory
    if (info.free_blocks == 0) handle_error("No free blocks available to create new directory");

    //now we scan the children of the parent directory to check if a directory with the same name already exists
    //read parent directory
    Entry* parent_dir = read_directory_from_block(disk_mem, parent_block, BLOCK_SIZE, disk_size_bytes);
    if (parent_dir == NULL) handle_error("Failed to read parent directory");
    
    //printf("Parent directory read successfully.\n");

    uint32_t* children_blocks = get_children_blocks(parent_dir);
    if (children_blocks == NULL) handle_error("Failed to get children blocks of parent directory");

    for (int i = 0; i < MAX_DIR_ENTRIES; i++) {
        if (children_blocks[i] == 0) break; // No more children
        Entry* child = read_directory_from_block(disk_mem, children_blocks[i], BLOCK_SIZE, disk_size_bytes);
        if (child == NULL) handle_error("Failed to read child directory");
        if (strcmp(child->name, name) == 0 && child->type == ENTRY_TYPE_DIR) {
            handle_error("Directory with the same name already exists in the parent directory");
        }
    }
    //if we reach here, it means we can create the new directory
    
    //printf("Parent directory read successfully.\n");
    //print_directory(parent_dir);

    //allocate block for new directory
    uint32_t new_dir_block = allocateBlock(fat, &info);
    if (new_dir_block == FAT_EOF) handle_error("Failed to allocate block for new directory");
    
    //initialize new directory
    Entry new_dir;
    init_directory(&new_dir, name, new_dir_block, ENTRY_TYPE_DIR);
    new_dir.current_block = new_dir_block;
    
    //write new directory to disk
    int res = write_directory(disk_mem, &new_dir, BLOCK_SIZE, disk_size_bytes);
    if (res != 0) handle_error("Failed to write new directory to disk");
    //printf("New directory written to disk successfully.\n");
    
    //add new directory to parent
    update_directory_children(parent_dir, new_dir_block);
    
    //write updated parent directory to disk
    res = write_directory(disk_mem, parent_dir, BLOCK_SIZE, disk_size_bytes);
    if (res != 0) handle_error("Failed to update parent directory on disk");
    //printf("Parent directory updated successfully.\n");
    
    //update fat and metainfo on disk
    res = update_fat_and_metainfo(disk_mem, fat, num_fat_entries, 1, &info, BLOCK_SIZE, disk_size_bytes);
    if (res != 0) handle_error("Failed to update FAT and metainfo after creating new directory");
    //printf("FAT and metainfo updated successfully after creating new directory.\n");
    
    //printf("Directory '%s' created successfully inside parent block %u\n", name, parent_block);
    
    //we can print the updated parent directory
    //printf("Updated parent directory:\n");
    //print_directory(parent_dir);
    
    //we can print the new directory
    //printf("New directory:\n");
    //print_directory(&new_dir);
    
    //print updated disk info and fat
    //print_disk_info(&info);
    //print_fat(fat, 10);
    
    //clean up
    free(parent_dir);
}

void remove_directory(char* disk_mem, const char *name, uint32_t parent_block, size_t disk_size_bytes) {
    //we need to find the child directory with the given name and remove it from its parent
    //we need to load metainfo and fat first
    DiskInfo info;
    uint32_t num_fat_entries = disk_size_bytes / BLOCK_SIZE;
    uint32_t fat[num_fat_entries];
    load_info_and_fat(disk_mem, &info, fat, disk_size_bytes);
    //now that we have metainfo and fat in RAM, we can proceed
    // read parent directory
    Entry* parent_dir = read_directory_from_block(disk_mem, parent_block, BLOCK_SIZE, disk_size_bytes);
    if (parent_dir == NULL) handle_error("Failed to read parent directory");
    // find the child directory to remove
    uint32_t* children_blocks = get_children_blocks(parent_dir);
    if (children_blocks == NULL) handle_error("Failed to get children blocks of parent directory");
    int dir_index = -1;
    Entry* dir_to_remove = NULL;
    for (int i = 0; i < MAX_DIR_ENTRIES; i++) {
        if (children_blocks[i] == 0) break; // No more children
        Entry* child = read_directory_from_block(disk_mem, children_blocks[i], BLOCK_SIZE, disk_size_bytes);
        if (child == NULL) handle_error("Failed to read child directory");
        if (strcmp(child->name, name) == 0 && child->type == ENTRY_TYPE_DIR) {
            dir_index = i;
            dir_to_remove = child;
            break;
        }
    }
    if (dir_index == -1 || dir_to_remove == NULL) {
        handle_error("Directory to remove not found in parent directory");
    }
    // check if directory is empty
    if (dir_to_remove->size > 1) {
        handle_error("Directory is not empty, cannot remove");
    }
    // deallocate directory block
    int res = deallocateChain(fat, &info, dir_to_remove->current_block);
    if (res != 0) handle_error("Failed to deallocate directory block");
    // remove directory from parent by zeroing its entry
    parent_dir->dir_blocks[dir_index] = 0;
    parent_dir->size--;
    // write updated parent directory to disk
    res = write_directory(disk_mem, parent_dir, BLOCK_SIZE, disk_size_bytes);
    if (res != 0) handle_error("Failed to write updated parent directory to disk");
    // update fat and metainfo on disk
    res = update_fat_and_metainfo(disk_mem, fat, num_fat_entries, 1, &info, BLOCK_SIZE, disk_size_bytes);
    if (res != 0) handle_error("Failed to update FAT and metainfo after removing directory");
}

void list_directory_contents(char* disk_mem, uint32_t cursor, size_t disk_size_bytes) {
    //read directory at cursor
    Entry* dir = read_directory_from_block(disk_mem, cursor, BLOCK_SIZE, disk_size_bytes);
    if (dir == NULL) handle_error("Failed to read directory at cursor");

    printf("Contents of directory '%s':\n", dir->name);
    uint32_t* children_blocks = get_children_blocks(dir);
    
    if (children_blocks == NULL) handle_error("Failed to get children blocks of directory");
    
    for (int i = 0; i < MAX_DIR_ENTRIES; i++) {
        if (children_blocks[i] == 0) break; // No more children
        
        Entry* child = read_directory_from_block(disk_mem, children_blocks[i], BLOCK_SIZE, disk_size_bytes);
        if (child == NULL) handle_error("Failed to read child entry");
        printf("- Name: %s - Type: %s - Size: %u bytes\n", child->name, child->type == ENTRY_TYPE_DIR ? "Directory" : "File", child->size*BLOCK_SIZE);
        free(child);
    }
    free(dir);

}