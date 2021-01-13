#include "FileSystem.h"
#include "SharedFiles.h"

#include <WASI/WASIPrivate.h>
#include <boost/filesystem.hpp>
#include <faabric/util/config.h>
#include <faabric/util/logging.h>

namespace storage {
void FileSystem::prepareFilesystem()
{
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

void FileSystem::createPreopenedFileDescriptor(int fd, const std::string& path)
{
    // Open the descriptor as a directory
    storage::FileDescriptor fileDesc;
    fileDesc.setPath(path);
    fileDesc.setActualRights(DIRECTORY_RIGHTS, INHERITING_DIRECTORY_RIGHTS);
    bool success = fileDesc.pathOpen(0, __WASI_O_DIRECTORY, 0);

    if (!success) {
        const std::string realSysPath = prependRuntimeRoot(path);
        faabric::util::getLogger()->error(
          "Failed on preopened fd at {} (system path {}). Error: {} ",
          path,
          realSysPath,
          strerror(fileDesc.getLinuxErrno()));
        throw std::runtime_error("Problem opening preopened fd");
    } else {
        faabric::util::getLogger()->debug("Opened preopened fd at {}", path);
    }

    // Add to this module's fds
    fileDesc.wasiPreopenType = __WASI_PREOPENTYPE_DIR;
    fileDescriptors.emplace(fd, fileDesc);
}

int FileSystem::getNewFd()
{
    // Assign a new file descriptor
    int thisFd = nextFd;
    nextFd++;
    return thisFd;
}

int FileSystem::openFileDescriptor(int rootFd,
                                   const std::string& relativePath,
                                   uint64_t rightsBase,
                                   uint64_t rightsInheriting,
                                   uint32_t lookupFlags,
                                   uint32_t openFlags,
                                   int32_t fdFlags)
{

    storage::FileDescriptor& rootFileDesc = getFileDescriptor(rootFd);

    std::string fullPath;
    if (SharedFiles::isPathShared(relativePath)) {
        fullPath = relativePath;
    } else if (rootFileDesc.getPath() == ".") {
        fullPath = relativePath;
    } else {
        boost::filesystem::path joinedPath(rootFileDesc.getPath());
        joinedPath.append(relativePath);
        fullPath = std::string(joinedPath.string());
    }

    int thisFd = getNewFd();
    FileDescriptor& fileDesc = fileDescriptors[thisFd];
    fileDesc.setPath(fullPath);

    // Set rights on new file descriptor
    uint64_t effectiveRights = rightsBase & rootFileDesc.getActualRightsBase();
    uint64_t effectiveInheritedRights =
      rightsInheriting & rootFileDesc.getActualRightsInheriting();
    fileDesc.setActualRights(effectiveRights, effectiveInheritedRights);

    // Open the path
    bool success = fileDesc.pathOpen(lookupFlags, openFlags, fdFlags);
    if (!success) {
        return -1 * fileDesc.getWasiErrno();
    }

    return thisFd;
}

bool FileSystem::fileDescriptorExists(int fd)
{
    return fileDescriptors.count(fd) > 0;
}

storage::FileDescriptor& FileSystem::getFileDescriptor(int fd)
{
    if (fileDescriptors.count(fd) == 0) {
        throw std::runtime_error("File descriptor does not exist");
    }

    return fileDescriptors.at(fd);
}

int FileSystem::dup(int fd)
{
    // We just want to map the *same* underlying object to a new descriptor
    int thisFd = getNewFd();

    FileDescriptor& originalDesc = getFileDescriptor(fd);
    fileDescriptors[thisFd] = originalDesc;

    return thisFd;
}

void FileSystem::tearDown()
{
    for (auto& f : fileDescriptors) {
        // Only close non-preopened fds
        if (f.second.wasiPreopenType != __WASI_PREOPENTYPE_DIR) {
            f.second.close();
        }
    }
}

void FileSystem::clearSharedFiles()
{
    SharedFiles::clear();

    // Just nuke the whole shared directory
    // Note that we're not deleting the master copies in the shared store
    faabric::util::SystemConfig& conf = faabric::util::getSystemConfig();
    boost::filesystem::remove_all(conf.sharedFilesDir);
}

}
