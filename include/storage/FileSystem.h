#pragma once

#include "FileDescriptor.h"

#include <proto/faasm.pb.h>

#include <unordered_map>

namespace storage {
    class FileSystem {
    public:
        void prepareFilesystem(const message::Message &msg);

        bool fileDescriptorExists(int fd);

        storage::FileDescriptor &getFileDescriptor(int fd);

        int openFileDescriptor(int rootFd, const std::string &path,
                               uint64_t rightsBase, uint64_t rightsInheriting, uint32_t openFlags);

        void createPreopenedFileDescriptor(int fd, const std::string &path);

    private:
        int nextFd;
        std::unordered_map<int, storage::FileDescriptor> fileDescriptors;
    };
}