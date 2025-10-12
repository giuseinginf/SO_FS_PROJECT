This is a simple Shell + File System implementation project.

The project is entirely written in C language and it's part of the Operating Systems Course for Software Engineering Bachelor Degree at Sapienza University of Rome. The course is taught by professor Giorgio Grisetti.

The file system is written on a large file ("disk"), which is opened, truncated and mapped into memory. The disk is accessed in blocks. On startup, the user is asked the size (16 MB, 32 MB or 64 MB). Each disk block is 4 kb. The disk is mapped into memory throuhg the "mmap" function. It returns a pointer which allows the user to access the disk the same way arrays are accessed.

Each file block index is stored in a File Allocation Table (FAT), which acts as a linked list for the blocks that make up a file. The FAT is basically an array list that keeps track of which blocks are free, which are in use, and the chain of blocks for each file.

The free space is managed through a specific index, the free list head, which keeps track of the free blocks. Even in this case, the index points to a block having the next one's index. The free list is in the FAT itself.

When a file is created or extended, the FAT is updated to reflect the allocation of new blocks. To read or write a file, the system follows the chain of block indexes in the FAT, allowing efficient traversal and management of file data.

Each file entry can be a file or a directory. If it's a file, it contains the File Control Block. If it's a drectory, it contains a list of entries.