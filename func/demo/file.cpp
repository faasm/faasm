#include "faasm/faasm.h"

#include <cstdio>
#include <cerrno>
#include <cstring>
#include <cstdlib>

#include <unistd.h>

#define FILE_NAME "dummy_file.txt"
#define FILE_CONTENTS "blah blah blah"
#define BUFFER_SIZE 15

FILE *openFile(const char *mode) {
    FILE *fptr;
    fptr = fopen(FILE_NAME, mode);
    if (fptr == nullptr) {
        printf("Failed to open %s: %i (%s)", FILE_NAME, errno, strerror(errno));
        exit(1);
    }

    return fptr;
}

void deleteFile() {
    // Delete file if it exists
    if (access(FILE_NAME, F_OK) != -1) {
        int removeRes = remove(FILE_NAME);
        if (removeRes != 0) {
            printf("Failed to remove %s: %i (%s)\n", FILE_NAME, errno, strerror(errno));
            exit(1);
        };
    }

}

FAASM_MAIN_FUNC() {
    // Initial delete
    deleteFile();

    // Open and write data
    FILE *fA = openFile("w");
    fprintf(fA, "%s\n", FILE_CONTENTS);
    fclose(fA);

    // Open and read data
    char buffer[BUFFER_SIZE];
    FILE *fB = openFile("r");

    char *getsRes = fgets(buffer, BUFFER_SIZE, fB);
    if (getsRes == nullptr) {
        printf("fgets failed\n");
        exit(1);
    }

    if (getsRes != buffer) {
        printf("getsRes %p != buffer %p\n", getsRes, buffer);
        exit(1);
    }

    if (strcmp(buffer, FILE_CONTENTS) != 0) {
        printf("Read in contents not as expected (actual=\"%s\" expected=\"%s\"\n)", buffer, FILE_CONTENTS);
        exit(1);
    }

    // Delete again
    deleteFile();

    return 0;
}

