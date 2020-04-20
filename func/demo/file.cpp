#include "faasm/faasm.h"

#include <cstdio>
#include <cerrno>
#include <cstring>
#include <cstdlib>

#include <unistd.h>

#define FILE_CONTENTS "blah blah blah"
#define BUFFER_SIZE 15

FILE *openFile(const char* fileName, const char *mode) {
    FILE *fptr;
    fptr = fopen(fileName, mode);
    if (fptr == nullptr) {
        printf("Failed to open %s: %i (%s)", fileName, errno, strerror(errno));
        exit(1);
    }

    return fptr;
}

void deleteFile(const char* fileName) {
    // Delete file if it exists
    if (access(fileName, F_OK) != -1) {
        int removeRes = remove(fileName);
        if (removeRes != 0) {
            printf("Failed to remove %s: %i (%s)\n", fileName, errno, strerror(errno));
            exit(1);
        };
    }
}

void checkFileContents(const char* fileName) {
    char buffer[BUFFER_SIZE];
    FILE *fA2 = openFile(fileName, "r");

    char *getsRes = fgets(buffer, BUFFER_SIZE, fA2);
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
}

FAASM_MAIN_FUNC() {
    const char* fileA = "dummy_file.txt";
    const char* fileB = "other_dummy_file.txt";

    // Initial delete
    deleteFile(fileA);
    deleteFile(fileB);

    // Open and write data
    FILE *fA1 = openFile(fileA, "w");
    fprintf(fA1, "%s\n", FILE_CONTENTS);
    fclose(fA1);

    // Check file
    checkFileContents(fileA);

    // Rename file
    int renameRes = rename(fileA, fileB);
    if(renameRes != 0) {
        printf("Rename failed (from=%s to %s, %i (%s))", fileA, fileB, errno, strerror(errno));
        exit(1);
    }

    // Check renamed file contents
    checkFileContents(fileB);

    // Delete again
    deleteFile(fileA);
    deleteFile(fileB);

    return 0;
}

