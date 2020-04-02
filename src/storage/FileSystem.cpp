#include "FileSystem.h"

#include <WASI/WASIPrivate.h>
#include <boost/filesystem.hpp>

namespace storage {
    void FileSystem::prepareFilesystem(const message::Message &msg) {
        // Predefined stdin, stdout and stderr
        fileDescriptors.emplace(0, storage::FileDescriptor::stdinFactory());
        fileDescriptors.emplace(1, storage::FileDescriptor::stdoutFactory());
        fileDescriptors.emplace(2, storage::FileDescriptor::stderrFactory());

        // Add roots, note that they are predefined as the file descriptors
        // just above the stdxxx's (i.e. > 3)
        createPreopenedFileDescriptor(3, "/");
        createPreopenedFileDescriptor(4, ".");

        // Set the starting point for subsequent file descriptors
        nextFd = 5;
    }

    void FileSystem::createPreopenedFileDescriptor(int fd, const std::string &path) {
        // Open the descriptor as a directory
        storage::FileDescriptor fileDesc(path);
        fileDesc.path_open(
                DIRECTORY_RIGHTS,
                INHERITING_DIRECTORY_RIGHTS,
                0
        );

        // Add to this module's fds
        fileDesc.wasiPreopenType = __WASI_PREOPENTYPE_DIR;
        fileDescriptors.emplace(fd, fileDesc);
    }

    int FileSystem::openFileDescriptor(int rootFd, const std::string &path,
                                       uint64_t rightsBase, uint64_t rightsInheriting, uint32_t openFlags) {
        storage::FileDescriptor &rootFileDesc = getFileDescriptor(rootFd);

        std::string fullPath;
        if(rootFileDesc.path == ".") {
            fullPath = path;
        } else {
            boost::filesystem::path joinedPath(rootFileDesc.path);
            joinedPath.append(path);
            fullPath = std::string(joinedPath.string());
        }

        // Assign a new file descriptor
        int thisFd = nextFd;
        nextFd++;
        fileDescriptors.try_emplace(thisFd, fullPath);
        storage::FileDescriptor &fileDesc = fileDescriptors.at(thisFd);

        int linuxFd = fileDesc.path_open(rightsBase, rightsInheriting, openFlags);
        if(linuxFd < 0) {
            return linuxFd;
        }

        return thisFd;
    }

    bool FileSystem::fileDescriptorExists(int fd) {
        return fileDescriptors.count(fd) > 0;
    }

    storage::FileDescriptor &FileSystem::getFileDescriptor(int fd) {
        if (fileDescriptors.count(fd) == 0) {
            throw std::runtime_error("File descriptor does not exist");
        }

        return fileDescriptors.at(fd);
    }
}