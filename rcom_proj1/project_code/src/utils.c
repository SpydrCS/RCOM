#include <sys/stat.h>

#include "../include/utils.h"

int get_file_size(const char *filename) {
    struct stat st;
    size_t file_size = 0;
    
    if (stat(filename, &st) < 0) {
        LOG_AL_ERROR("ERROR: get_file_size() in utils.c\n")
        return -1; 
    } else {
        file_size = st.st_size;
    }

    return file_size;
}
