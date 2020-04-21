#include "faasm/faasm.h"

#include <fcntl.h>
#include <stdio.h>
#include <cerrno>
#include <cstring>
#include <unistd.h>
#include <cstdlib>

#define FILE_PATH "/etc/hosts"

FAASM_MAIN_FUNC() {
    int fd = open(FILE_PATH, O_RDONLY);

    if (fd <= 0) {
        printf("Unable to open file %s\n", FILE_PATH);
        return 1;
    }

    // Duplicate the fd
    int newFd = fcntl(fd, F_DUPFD, 0);
    if (newFd <= 0) {
        printf("Unable to duplicate fd with fcntl (%i): %i (%s)\n", newFd, errno, strerror(errno));
        return 1;
    }

    if(newFd == fd) {
        printf("Expected dup-ed and original fd to be different\n");
        return 1;
    } else {
        printf("dup-ed = %i original = %i\n", newFd, fd);
    }

    char bufferA[100];
    ssize_t readBytesA = read(fd, bufferA, 100);
    if (readBytesA <= 0) {
        printf("Failed original fd (%li bytes, %i - %s)\n", readBytesA, errno, strerror(errno));
        return 1;
    }

    // Need to reset dup-ed as they point to the same underlying object
    lseek(fd, 0, SEEK_SET);
    lseek(newFd, 0, SEEK_SET);

    char bufferB[100];
    ssize_t readBytesB = read(newFd, bufferB, 100);
    if (readBytesB <= 0) {
        printf("Failed dup-ed fd (%li bytes, %i - %s)\n", readBytesB, errno, strerror(errno));
        return 1;
    }

    // Check output is as expected
    if (strcmp(bufferA, bufferB) == 0) {
        printf("Contents of %s from dup match\n", FILE_PATH);
    } else {
        printf("Contents of %s from dup don't match (%s vs %s)\n", FILE_PATH, bufferA, bufferB);
        return 1;
    }

    return 0;
}
