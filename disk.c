/*
Here we implement the disk API functions.
*/

#include "disk.h"

//Initialize disk
int  disk_init(Disk* d, const char* path, size_t disk_size, uint32_t block_size){
    if(d == NULL || path == NULL) {
        perror("Invalid input");
        exit(EXIT_FAILURE);
    }
    if(block_size == 0 || disk_size == 0 || disk_size % block_size != 0) {
        perror("Invalid block size or size");
        exit(EXIT_FAILURE);
    }
    if(disk_size/block_size > UINT32_MAX){
        perror("Invalid number of blocks: too many");
        exit(EXIT_FAILURE);
    }
    d->nblocks = (uint32_t)(disk_size / block_size);
    
    //open file
    int fd = open(path, O_RDWR | O_CREAT, 0666);
    if(fd < 0){
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }
    //resize file
    if(ftruncate(fd, disk_size) < 0) {
        perror("Failed to resize file");
        close(fd);
        exit(EXIT_FAILURE);
    }
    //save info
    d->fd = fd;
    d->disk_size = disk_size;
    d->block_size = block_size;
    d->nblocks = (uint32_t)(disk_size / block_size);

    //map file into memory
    d->base = mmap(NULL, disk_size, PROT_READ | PROT_WRITE, MAP_SHARED, d->fd, 0);
    if(d->base == MAP_FAILED) {
        perror("Failed to map file into memory");
        close(fd);
        d->fd = -1;
        d->base = NULL;
        exit(EXIT_FAILURE);
    }

    d->base = (uint8_t*) d->base;
    printf("DISK: function disk_init completed successfully\n");
    return 0;
}

//open disk if it exists
int disk_open(Disk* d, const char* path) {
    if(d == NULL){
        perror("Invalid disk");
        exit(EXIT_FAILURE); // Invalid input
    }
    if(path == NULL){
        perror("Invalid path");
        exit(EXIT_FAILURE);
    }
    if(d->block_size == 0){
        perror("Invalid block size");
        exit(EXIT_FAILURE);
    }
    //open existing file
    int fd = open(path, O_RDWR);
    if(fd < 0){
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }
    //recover file size
    struct stat st;
    if(fstat(fd, &st) < 0) {
        perror("Failed to get file status");
        close(fd);
        exit(EXIT_FAILURE);
    }
    d->disk_size = st.st_size;
    if(d->disk_size <= 0) {
        perror("Invalid disk size");
        close(fd);
        exit(EXIT_FAILURE);
    }
    //verify structure
    if(d->disk_size % d->block_size != 0) {
        perror("Invalid disk structure");
        close(fd);
        exit(EXIT_FAILURE);
    }
    d->nblocks = (uint32_t)(d->disk_size / d->block_size);
    if(d->nblocks == 0) {
        perror("Invalid number of blocks");
        close(fd);
        exit(EXIT_FAILURE);
    }
    //mmap file into memory
    d->base = mmap(NULL, d->disk_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(d->base == MAP_FAILED) {
        perror("Failed to map file into memory");
        close(fd);
        exit(EXIT_FAILURE);
    }
    //save info
    d->fd = fd;
    d->base = (uint8_t*) d->base;
    d->disk_size = d->disk_size;
    d->block_size = d->block_size;
    d->nblocks = d->nblocks;

    printf("DISK: function disk_open completed successfully\n");
    return 0;
}

bool offset_is_legal(size_t offset, size_t disk_size, uint32_t block_size){
    return (offset + block_size < disk_size);
}

void* disk_read(const Disk* d, uint32_t blkno){
    void* out_buf = NULL;
    if(!d){
        perror("Invalid disk");
        exit(EXIT_FAILURE);
    }
    if(blkno >= d->nblocks) {
        perror("Invalid block number: out of range");
        exit(EXIT_FAILURE);
    }
    size_t offset = (size_t)blkno * d->block_size;
    out_buf = malloc(d->block_size);
    if(!out_buf) {
        perror("Failed to allocate memory");
        exit(EXIT_FAILURE);
    }
    if(!offset_is_legal(offset, d->disk_size, d->block_size)) {
        perror("Read exceeds disk size");
        free(out_buf);
        exit(EXIT_FAILURE);
    }
    memcpy(out_buf, d->base + offset, d->block_size);
    printf("DISK: disk_read completed successfully\n");
    return out_buf;
}

void disk_write(const Disk* d, uint32_t blkno, const void* in_buf){

    if(!d){
        perror("Invalid disk");
        exit(EXIT_FAILURE);
    }

    if(!in_buf){
        perror("Invalid buffer");
        exit(EXIT_FAILURE);
    }

    if(blkno >= d->nblocks) {
        perror("Invalid block number: out of range");
        exit(EXIT_FAILURE);
    }

    size_t offset = (size_t)blkno * d->block_size;
    if(!offset_is_legal(offset, d->disk_size, d->block_size)) {
        perror("Write exceeds disk size");
        exit(EXIT_FAILURE);
    }
    memcpy(d->base + offset, in_buf, d->block_size);
    printf("DISK: disk_write completed successfully\n");
}

//close disk
int disk_close(Disk* d) {
    if(!d) {
        perror("Invalid disk");
        exit(EXIT_FAILURE);
    }
    if(d->base != NULL) {
        munmap(d->base, d->disk_size);
    }
    if(d->fd >= 0) {
        close(d->fd);
    }

    printf("DISK: function disk_close completed successfully\n");
    return 0;
}