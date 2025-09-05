This is a simple Shell + File System implementation project. It's written in C language and it'spart of the Operating System Course for Software Engineering bachelor degree at Sapienza University of Rome. The course is taught by professor Giorgio Grisetti.
The file system is written on a large file ("disk"), which is opened, truncated and mapped into memory.
The file allocation is implemented through a linked list.
Directories are linear lists of files. The hierarchy is preserved thanks to a tree structure.
The free space is managed through a linked list, where every bit corresponds to a specific free block.