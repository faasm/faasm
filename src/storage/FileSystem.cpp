#include "FileSystem.h"

#include <WASI/WASIPrivate.h>
#include <boost/filesystem.hpp>
#include <util/logging.h>
#include <util/config.h>

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
        fileDesc.setActualRights(DIRECTORY_RIGHTS, INHERITING_DIRECTORY_RIGHTS);
        bool success = fileDesc.pathOpen(0, __WASI_O_DIRECTORY, 0);

        if (!success) {
            util::getLogger()->error("Failed on preopened FD {} ({})", path, strerror(fileDesc.getLinuxErrno()));
            throw std::runtime_error("Problem opening preopened fd");
        }

        // Add to this module's fds
        fileDesc.wasiPreopenType = __WASI_PREOPENTYPE_DIR;
        fileDescriptors.emplace(fd, fileDesc);
    }

    int FileSystem::openFileDescriptor(int rootFd, const std::string &path,
                                       uint64_t rightsBase, uint64_t rightsInheriting,
                                       uint32_t lookupFlags, uint32_t openFlags, int32_t fdFlags) {

        storage::FileDescriptor &rootFileDesc = getFileDescriptor(rootFd);

        std::string fullPath;
        if (rootFileDesc.path == ".") {
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

        // Set rights on new file descriptor
        uint64_t effectiveRights = rightsBase & rootFileDesc.getActualRightsBase();
        uint64_t effectiveInheritedRights = rightsInheriting & rootFileDesc.getActualRightsInheriting();
        fileDesc.setActualRights(effectiveRights, effectiveInheritedRights);

        // Open the path
        bool success = fileDesc.pathOpen(lookupFlags, openFlags, fdFlags);
        if (!success) {
            return -1 * fileDesc.getWasiErrno();
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