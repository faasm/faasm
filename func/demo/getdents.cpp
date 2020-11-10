#include "faasm/faasm.h"

#include <dirent.h>
#include <faasm/input.h>
#include <stdio.h>
#include <string>
#include <sys/types.h>
#include <vector>

int main(int argc, char* argv[])
{
    const char* dirName = "/etc/";
    printf("Opening dir %s\n", dirName);

    DIR* dirp = opendir(dirName);

    if (dirp == nullptr) {
        printf("Couldn't open dir %s\n", dirName);
        return 1;
    }

    struct dirent* dp;
    std::string output;

    printf("ino     d_type  name\n");

    int count = 0;
    while ((dp = readdir(dirp)) != NULL) {
        ino_t d_ino = dp->d_ino;
        unsigned char d_type = dp->d_type;
        char* name = dp->d_name;

        output += name + std::string(",");

        printf("%u %u       %s\n", (unsigned int)d_ino, d_type, name);
        count++;
    }

    closedir(dirp);

    faasmSetOutput(reinterpret_cast<const uint8_t*>(output.c_str()),
                   output.size());

    return 0;
}
