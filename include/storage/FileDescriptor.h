#pragma once

#include <string>
#include <dirent.h>

namespace storage {
    class DirEnt {
    public:
        unsigned int next;
        uint8_t type;
        unsigned int ino;
        std::string path;
        bool isEnd = false;
    };

    class Stat {
    public:
        bool failed;
        uint16_t wasiErrno;
        uint8_t wasiFiletype;
    };

    class FileDescriptor {
    public:
        explicit FileDescriptor(std::string pathIn);

        DirEnt iterNext();

        Stat stat();

        int open(uint64_t rightsBaseIn, uint64_t rightsInheritingIn, uint32_t openFlags);

        void close();

        uint16_t seek(uint64_t offset, int whence, uint64_t *newOffset);

        const std::string path;
        bool iterStarted;
        bool iterFinished;

        uint64_t rightsBase;
        uint64_t rightsInheriting;

        int getLinuxFd();

    private:
        DIR *dirPtr;
        struct dirent *direntPtr;

        int linuxFd;
        int linuxMode;
        int linuxFlags;
    };
}

