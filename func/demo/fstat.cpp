#include "faasm/faasm.h"

#include <sys/stat.h>

#include <stdio.h>

namespace faasm {

    void compareSwithS64(struct stat &s, struct stat64 &s64) {
        #if WASM_BUILD == 1
        printf("st_dev: %llu\n", s.st_dev);
        printf("st_dev: %llu\n", s64.st_dev);

        printf("st_ino: %llu\n", s.st_ino);
        printf("st_ino: %llu\n", s64.st_ino);

        printf("st_nlink: %llu\n", s.st_nlink);
        printf("st_nlink: %llu\n", s64.st_nlink);
        #else
        printf("st_dev: %lu\n", s.st_dev);
        printf("st_dev: %lu\n", s64.st_dev);

        printf("st_ino: %lu\n", s.st_ino);
        printf("st_ino: %lu\n", s64.st_ino);

        printf("st_nlink: %lu\n", s.st_nlink);
        printf("st_nlink: %lu\n", s64.st_nlink);
        #endif

        printf("st_mode: %i\n", s.st_mode);
        printf("st_mode: %i\n", s64.st_mode);

        printf("st_uid: %i\n", s.st_uid);
        printf("st_uid: %i\n", s64.st_uid);

        printf("st_gid: %i\n", s.st_gid);
        printf("st_gid: %i\n", s64.st_gid);

        #if WASM_BUILD == 1
        printf("st_rdev: %llu\n", s.st_rdev);
        printf("st_rdev: %llu\n", s64.st_rdev);

        printf("st_size: %lli\n", s.st_size);
        printf("st_size: %lli\n", s64.st_size);
        #else
        printf("st_rdev: %lu\n", s.st_rdev);
        printf("st_rdev: %lu\n", s64.st_rdev);

        printf("st_size: %li\n", s.st_size);
        printf("st_size: %li\n", s64.st_size);
        #endif

        printf("st_blksize: %li\n", s.st_blksize);
        printf("st_blksize: %li\n", s64.st_blksize);
        #if WASM_BUILD == 1
        printf("st_blocks: %lli\n", s.st_blocks);
        printf("st_blocks: %lli\n", s64.st_blocks);
        #else
        printf("st_blocks: %li\n", s.st_blocks);
        printf("st_blocks: %li\n", s64.st_blocks);
        #endif

        #if WASM_BUILD == 1
        printf("st_atim.tv_sec: %lli\n", s.st_atim.tv_sec);
        printf("st_atim.tv_sec: %lli\n", s64.st_atim.tv_sec);

        printf("st_mtim.tv_sec: %lli\n", s.st_mtim.tv_sec);
        printf("st_mtim.tv_sec: %lli\n", s64.st_mtim.tv_sec);

        printf("st_ctim.tv_sec: %lli\n", s.st_ctim.tv_sec);
        printf("st_ctim.tv_sec: %lli\n", s64.st_ctim.tv_sec);
        #else
        printf("st_atim.tv_sec: %li\n", s.st_atim.tv_sec);
        printf("st_atim.tv_sec: %li\n", s64.st_atim.tv_sec);

        printf("st_mtim.tv_sec: %li\n", s.st_mtim.tv_sec);
        printf("st_mtim.tv_sec: %li\n", s64.st_mtim.tv_sec);

        printf("st_ctim.tv_sec: %li\n", s.st_ctim.tv_sec);
        printf("st_ctim.tv_sec: %li\n", s64.st_ctim.tv_sec);
        #endif
    }

    int exec(FaasmMemory *memory) {

        struct stat s{};
        struct stat64 s64{};

        fstat(fileno(stdin), &s);
        fstat64(fileno(stdin), &s64);

        compareSwithS64(s, s64);

        return 0;
    }
}
