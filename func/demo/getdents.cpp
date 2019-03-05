#include "faasm/faasm.h"

#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>

namespace faasm {
    int exec(FaasmMemory *memory) {
        const char *dirName = "/";

        DIR *dirp = opendir(dirName);

        struct dirent *dp;
        while ((dp = readdir(dirp)) != NULL) {
            printf("%s\n", dp->d_name);
        }

        closedir(dirp);

        return 0;
    }
}
