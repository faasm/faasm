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

    class FileDescriptor {
    public:
        explicit FileDescriptor(std::string pathIn);

        DirEnt iterNext();

        const std::string path;
        bool iterStarted;
        bool iterFinished;
    private:
        DIR *dirPtr;
        struct dirent *direntPtr;
    };
}

