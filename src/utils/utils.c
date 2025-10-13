#include "../utils/utils.h"

// error handling function
void handle_error(const char* msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

// Format size in bytes as KB/MB/GB string
const char* format_size(size_t bytes) {
    static char buf[32];
    if (bytes < 1024)
        snprintf(buf, sizeof(buf), "%zu bytes", bytes);
    else if (bytes < 1024 * 1024)
        snprintf(buf, sizeof(buf), "%.2f KB", bytes / 1024.0);
    else if (bytes < 1024 * 1024 * 1024)
        snprintf(buf, sizeof(buf), "%.2f MB", bytes / (1024.0 * 1024.0));
    else
        snprintf(buf, sizeof(buf), "%.2f GB", bytes / (1024.0 * 1024.0 * 1024.0));
    return buf;
}