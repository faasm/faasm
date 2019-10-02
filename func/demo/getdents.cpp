#include "faasm/faasm.h"

#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <vector>
#include <string>
#include <faasm/input.h>

FAASM_MAIN_FUNC() {
    const char* dirName = faasm::getStringInput("");
    DIR *dirp = opendir(dirName);
    if (dirp == nullptr) {
        printf("Couldn't open dir %s\n", dirName);
        return 1;
    }

    struct dirent *dp;
    std::string output;

    printf("ino        d_off            reclen   d_type   name\n");

    int count = 0;
    while ((dp = readdir(dirp)) != NULL) {
        ino_t d_ino = dp->d_ino;
        off_t off = dp->d_off;
        unsigned short reclen = dp->d_reclen;
        unsigned char d_type = dp->d_type;
        char *name = dp->d_name;

        output += name + std::string(",");

        printf("%u   %i   %i   %u   %s\n", (unsigned int) d_ino, (int) off, reclen, d_type, name);
        count++;
    }

    closedir(dirp);

    faasmSetOutput(reinterpret_cast<const uint8_t *>(output.c_str()), output.size());

    return 0;

}
