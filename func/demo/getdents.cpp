#include "faasm/faasm.h"

#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>

namespace faasm {
    void listDir() {
        const char *dirName = "/lib/python3.5";

        DIR *dirp = opendir(dirName);

        struct dirent *dp;
        while ((dp = readdir(dirp)) != NULL) {
            printf("%s\n", dp->d_name);
        }

        closedir(dirp);
    }
    int exec(FaasmMemory *memory) {
        listDir();
        listDir();
        return 0;
    }
}
