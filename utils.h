#pragma once
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <stdbool.h>
#include <stddef.h>

#define DEBUG 0

//error handling function
void handle_error(const char* msg);

// Format size in bytes as KB/MB/GB string
const char* format_size(size_t bytes);