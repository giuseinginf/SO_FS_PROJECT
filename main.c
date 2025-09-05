#include "disk.h"
#include "shell.h"
#include <stdio.h>
#include <unistd.h>

#define BLOCK_SIZE 4096 // 4 KB = 4096 bytes
#define DISK_SIZE 1024 * 1024 * 32 // 32 MB = 32 * 1024 * 1024 bytes
#define NUM_BLOCKS (DISK_SIZE / BLOCK_SIZE) // 8192
#define FILENAME "disk_image.img"

int main() {
    Disk disk;

    //we create a disk
    if (disk_init(&disk, FILENAME, DISK_SIZE, BLOCK_SIZE) != 0) {
        fprintf(stderr, "Failed to initialize disk.\n");
        return 1;
    }

    printf("MAIN: Disk image initialized successfully.\n");
    
    //we print disk info
    printf("MAIN: Disk size: %lu bytes\n", disk.disk_size);
    printf("MAIN: Block size: %u bytes\n", disk.block_size);
    printf("MAIN: Number of blocks: %u\n", disk.nblocks);
    
    //we write something
    const char* test_data = "Hello, Disk!";
    disk_write(&disk, 0, test_data);

    //we close the disk
    disk_close(&disk);
    printf("MAIN: Disk image closed successfully.\n");
    
    printf("-------------------\n");

    //we reopen the disk
    if (disk_open(&disk, FILENAME) != 0) {
        fprintf(stderr, "Failed to open disk.\n");
        return 1;
    }

    printf("MAIN: Disk image opened successfully.\n");
    //we print disk info
    printf("MAIN: Disk size: %lu bytes\n", disk.disk_size);
    printf("MAIN: Block size: %u bytes\n", disk.block_size);
    printf("MAIN: Number of blocks: %u\n", disk.nblocks);

    //we read something
    char* read_data = disk_read(&disk, 0);
    if (read_data) {
        printf("MAIN: Read data: %s\n", read_data);
        free(read_data);
    }

    //we close the disk
    disk_close(&disk);
    printf("MAIN: Disk image closed successfully.\n");

    shell_init();

    return 0;
}