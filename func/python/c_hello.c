#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>

int main(int argc, char *argv[]) {
    printf("Hello from C\n");

    DIR *dirp = opendir("/lib/python3.5/encodings");

    struct dirent *dp;
    while ((dp = readdir(dirp)) != NULL) {
        printf("%s\n", dp->d_name);
    }

    closedir(dirp);

    return 0;
}