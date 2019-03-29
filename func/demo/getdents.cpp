#include "faasm/faasm.h"

#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>

namespace faasm {
    int listDir() {
        // Native versions commented out
        const char *dirName = "lib/python3.7";
//        const char *dirName = "/usr/lib";

        DIR *dirp = opendir(dirName);
        if(dirp == nullptr) {
            printf("Couldn't open dir %s", dirName);
            return 1;
        }

        struct dirent *dp;
        printf("ino        d_off            reclen   d_type   name\n");
        while ((dp = readdir(dirp)) != NULL) {
            ino_t d_ino = dp->d_ino;
            off_t off = dp->d_off;
            unsigned short reclen = dp->d_reclen;
            unsigned char d_type = dp->d_type;
            char *name256 = dp->d_name;

            printf("%u   %i   %i   %u   %s\n", (unsigned int) d_ino, (int) off, reclen, d_type, name256);
//            printf("%li   %li   %i   %u   %s\n", d_ino, off, reclen, d_type, name256);
        }

        closedir(dirp);

        return 0;
    }

    int exec(FaasmMemory *memory) {
        return listDir();
    }
}
