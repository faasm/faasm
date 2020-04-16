#include "faasm/files.h"

#include <stdio.h>

#include <fcntl.h>
#include <malloc.h>
#include <sys/stat.h>

namespace faasm {
    long getFileLength(const char *path) {
        int fd = open(path, O_RDONLY);
        if (fd == -1) {
            return -1;
        }

        struct stat s{};
        fstat(fd, &s);

        return s.st_size;
    }

    char *readFileToString(const char *path) {
        printf("RFTS - fopen %s\n", path);
        FILE *f = fopen(path, "rb");

        if (f == nullptr) {
            return nullptr;
        }

        printf("RFTS - getFileLength %s\n", path);
        long length = getFileLength(path);
        if (length == -1) {
            printf("RFTS - getFileLength %s - %li\n", path, length);
            return nullptr;
        }

        // Read in file
        size_t res = 0;
        auto buffer = (char *) malloc(length + 1);
        if (buffer) {
            printf("RFTS - fread %s \n", path);
            res = fread(buffer, 1, length, f);
            buffer[length] = '\0';
        }
        fclose(f);

        if (res == 0) {
            printf("RFTS - fread res %li \n", res);
            return nullptr;
        }

        return buffer;
    }
}