#include "disk.h"
#include "shell.h"

#include <stdio.h>
#include <unistd.h>

void test_function() {
    //init disk
    const char* disk_path = "virtual_disk.img";

    //size_t disk_size = 32 * 1024 * 1024; // 32 MB
    size_t disk_size = 16 * 1024 * 1024; // 16 MB
    uint32_t block_size = 4096; // 4 KB

    char* disk_memory = open_and_map_disk(disk_path, disk_size);
    if(!disk_memory) {
        perror("Failed to initialize disk");
        exit(EXIT_FAILURE);
    }
    printf("Disk initialized successfully. Name: %s, size: %zu bytes, blocks: %u bytes\n", disk_path, disk_size, block_size);

    //write to block 0
    const char* message = "Hello, World!";
    write_block(disk_memory, 0, message, block_size, disk_size);
    printf("Wrote to block 0: %s\n", message);

    //read from block 0
    char buffer[block_size];
    read_block(disk_memory, 0, buffer, block_size, disk_size);
    printf("Read from block 0: %s\n", buffer);

}

int main() {
    test_function();
//    shell_init();

    return 0;
}