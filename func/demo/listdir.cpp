#include "faasm/faasm.h"

#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <vector>
#include <string>
#include <errno.h>
#include <string.h>

#define NAMLEN(dirent) strlen((dirent)->d_name)

std::vector<std::string> listDir(const char *dirPath) {
    DIR *dirp = opendir(dirPath);
    struct dirent *ep;

    std::vector<std::string> results;

    for (;;) {
        errno = 0;
        ep = readdir(dirp);
        if (ep == NULL) {
            if (errno == 0) {
                break;
            } else {
                printf("readdir failed: %s\n", strerror(errno));
                results.clear();
                return results;
            }
        }

        if (ep->d_name[0] == '.' && (NAMLEN(ep) == 1 || (ep->d_name[1] == '.' && NAMLEN(ep) == 2))) {
            continue;
        }

        results.emplace_back(ep->d_name);
    }

    return results;
}

FAASM_MAIN_FUNC() {
    const char *dirA = "/lib/python3.7/multiprocessing/";
    const char *dirB = "/lib/python3.7/encodings/";

    std::vector<std::string> vecA = listDir(dirA);
    if (vecA.empty()) {
        printf("Error listing dir %s\n", dirA);
    }

    std::vector<std::string> vecB = listDir(dirB);
    if (vecB.empty()) {
        printf("Error listing dir %s\n", dirB);
    }

    printf("vecA = %lu\n", vecA.size());
    printf("vecB = %lu\n", vecB.size());

    return 0;
}