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
        FILE *f = fopen(path, "rb");

        if (f == nullptr) {
            return nullptr;
        }

        long length = getFileLength(path);
        if (length == -1) {
            return nullptr;
        }

        // Read in file
        auto buffer = (char *) malloc(length);
        if (buffer) {
            fread(buffer, 1, length, f);
        }
        fclose(f);

        return buffer;
    }
}