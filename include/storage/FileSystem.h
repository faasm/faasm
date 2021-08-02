#pragma once

#include "FileDescriptor.h"

#include <faabric/proto/faabric.pb.h>

#include <unordered_map>

namespace storage {
class FileSystem
{
  public:
    void prepareFilesystem();

    bool fileDescriptorExists(int fd);

    storage::FileDescriptor& getFileDescriptor(int fd);

    int openFileDescriptor(int rootFd,
                           const std::string& path,
                           uint64_t rightsBase,
                           uint64_t rightsInheriting,
                           uint32_t lookupFlags,
                           uint32_t openFlags,
                           int32_t fdFlags);

    void createPreopenedFileDescriptor(int fd, const std::string& path);

    int dup(int fd);

    void tearDown();

    std::string getPathForFd(int fd);

    void printDebugInfo();

  private:
    int nextFd;

    std::unordered_map<int, storage::FileDescriptor> fileDescriptors;

    int getNewFd();
};
}
