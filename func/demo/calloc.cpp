#include <stdio.h>
#include <malloc.h>
#include <dirent.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>


int main(int argc, char *argv[]) {
    const char *filename = "/lib/python3.7/multiprocessing/";
    int fd;
    DIR *dir;
    fd = open(filename, O_RDONLY | O_DIRECTORY | O_CLOEXEC);

    int callocSize = 2080;

    if (fd < 0) {
        printf("Open failed: %s\n", strerror(errno));
        return 1;
    }

    printf("Attempting to calloc (1, %i)\n", callocSize);

    dir = (DIR*) calloc(1, callocSize);
    if (!dir) {
        printf("Calloc failed: %s\n", strerror(errno));
    }

    printf("Calloc successful. dir = %i\n", dir);

    return 0;
}