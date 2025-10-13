# FS Shell - Simple File System & Shell Implementation

**FS Shell** is a simple shell environment and file system written in C, developed as part of the Operating Systems course for the Software Engineering Bachelor Degree at Sapienza University of Rome (Prof. Giorgio Grisetti).

---

## Overview

The project provides a minimal file system that operates on a single large file (the "disk"), which is mapped into memory via `mmap`. The disk is accessed and managed in fixed-size blocks (4 KB each). On startup, the user selects the disk size (16 MB, 32 MB, or 64 MB).

## How it works

- The disk is memory-mapped, allowing efficient random access to blocks.
- File and directory management is implemented on top of a custom File Allocation Table (FAT) structure.
- The FAT acts as a linked list, tracking which blocks are free, which are allocated, and the block chain for each file.
- Free space is managed via a "free list head" index, allowing efficient allocation and deallocation of blocks.
- File and directory entries include metadata and, in the case of directories, a list of contained entries.

## Features

- **Format disk:** Create and initialize a new disk image.
- **Create directories and files:** Hierarchical structure with unlimited depth (within block limits).
- **List contents:** Print files and subdirectories in the current directory.
- **Navigate:** Change directory (`cd`), including parent navigation (`..`).
- **File operations:** Create (`touch`), append data (`append`), display contents (`cat`), and remove files (`rm`).
- **Directory operations:** Create (`mkdir`) and remove (`rmdir`) directories, with checks for non-empty directories.
- **Persistence:** All changes are written to the disk image and persist across executions.

## Structure Overview

- **Disk image:** All data is stored in a single file, accessed and modified in blocks.
- **FAT:** The File Allocation Table is stored at the beginning of the disk, acting as the main index for block management.
- **Entries:** Each file or directory is represented by an entry structure. Files store control metadata; directories store the block indexes of their children.
- **Free List:** Free blocks are managed through a linked list embedded in the FAT.

## Example Usage

```sh
$ ./bin/fs-shell

SHELL:/$ format disk.img
Enter disk size in MB (16, 32, 64): 16

SHELL:/$ mkdir docs
SHELL:/$ cd docs
SHELL:/docs$ touch notes.txt
SHELL:/docs$ append notes.txt "Hello, filesystem!"
SHELL:/docs$ cat notes.txt
Hello, filesystem!
SHELL:/docs$ ls
- Name: notes.txt - Type: File - Size: 18 bytes
SHELL:/docs$ cd ..
SHELL:/$ ls
- Name: docs - Type: Directory - Size: 18 bytes
SHELL:/$ close
```

---

## Course & Credits

This project is part of the Operating Systems course for the Software Engineering Bachelor Degree at [Sapienza University of Rome](https://www.uniroma1.it/en) and is supervised by Professor Giorgio Grisetti.

---

## Notes

- The project code is modularized into multiple source files for clarity and maintainability.
- The design aims to illustrate OS concepts such as block management, FAT, directory trees, and file operations in a simplified but practical context.
