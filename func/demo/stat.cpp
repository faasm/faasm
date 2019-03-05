#include "faasm/faasm.h"

#include <sys/stat.h>

#include <stdio.h>

namespace faasm {
    int exec(FaasmMemory *memory) {

        struct stat64 s{};
        stat64("/", &s);

        printf("STAT: \n");
        printf("Host stat:\n");
        printf("st_dev: %li\n", s.st_dev);
        printf("st_mode: %i\n", s.st_mode);
        printf("st_nlink: %li\n", s.st_nlink);

        printf("st_uid: %i\n", s.st_uid);
        printf("st_gid: %i\n", s.st_gid);

        printf("st_rdev: %li\n", s.st_rdev);
        printf("st_size: %li\n", s.st_size);

        printf("st_blksize: %li\n", s.st_blksize);
        printf("st_blocks: %li\n", s.st_blocks);

        printf("st_atim.tv_sec: %li\n", s.st_atim.tv_sec);
        printf("st_mtim.tv_sec: %li\n", s.st_mtim.tv_sec);
        printf("st_ctim.tv_sec: %li\n", s.st_ctim.tv_sec);

        printf("st_ino: %li\n", s.st_ino);

        return 0;
    }
}
