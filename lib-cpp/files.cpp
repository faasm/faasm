#include "faasm/files.h"

#include <stdio.h>
#include <malloc.h>

namespace faasm {
    long getFileLength(const char *path) {
        FILE *f = fopen(path, "rb");

        if (f == nullptr) {
            return -1;
        }

        // Work out file length
        fseek(f, 0, SEEK_END);
        return ftell(f);
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