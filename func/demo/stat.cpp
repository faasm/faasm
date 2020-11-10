#include "faasm/faasm.h"

#include <sys/stat.h>

#include <stdio.h>

int main(int argc, char* argv[])
{

    struct stat64 s
    {};

    // NOTE: in the wasm world the struct has different types, so print whines
    // here if we don't branch
#ifdef __wasm__

    stat64("/", &s);
    printf("st_dev: %llu\n", s.st_dev);
    printf("st_ino: %llu\n", s.st_ino);
    printf("st_nlink: %llu\n", s.st_nlink);

    printf("st_mode: %i\n", s.st_mode);
    printf("st_uid: %i\n", s.st_uid);
    printf("st_gid: %i\n", s.st_gid);

    printf("st_rdev: %llu\n", s.st_rdev);
    printf("st_size: %lli\n", s.st_size);

    printf("st_blksize: %li\n", s.st_blksize);
    printf("st_blocks: %lli\n", s.st_blocks);

    printf("st_atim.tv_sec: %lli\n", s.st_atim.tv_sec);
    printf("st_mtim.tv_sec: %lli\n", s.st_mtim.tv_sec);
    printf("st_ctim.tv_sec: %lli\n", s.st_ctim.tv_sec);

#else
    stat64("/usr/local/faasm/runtime_root/", &s);

    printf("st_dev: %lu\n", s.st_dev);
    printf("st_ino: %lu\n", s.st_ino);
    printf("st_nlink: %lu\n", s.st_nlink);

    printf("st_mode: %i\n", s.st_mode);
    printf("st_uid: %i\n", s.st_uid);
    printf("st_gid: %i\n", s.st_gid);

    printf("st_rdev: %lu\n", s.st_rdev);
    printf("st_size: %li\n", s.st_size);

    printf("st_blksize: %li\n", s.st_blksize);
    printf("st_blocks: %li\n", s.st_blocks);

    printf("st_atim.tv_sec: %li\n", s.st_atim.tv_sec);
    printf("st_mtim.tv_sec: %li\n", s.st_mtim.tv_sec);
    printf("st_ctim.tv_sec: %li\n", s.st_ctim.tv_sec);
#endif

    return 0;
}
