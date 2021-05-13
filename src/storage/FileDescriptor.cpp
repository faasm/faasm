#include "FileDescriptor.h"

#include <faabric/util/bytes.h>
#include <faabric/util/config.h>
#include <faabric/util/logging.h>
#include <faabric/util/macros.h>
#include <faabric/util/timing.h>

#include <conf/FaasmConfig.h>
#include <storage/SharedFiles.h>

#include <WAVM/WASI/WASIABI.h>
#include <boost/filesystem.hpp>
#include <dirent.h>
#include <fcntl.h>
#include <stdexcept>
#include <sys/stat.h>

#define WASI_FD_FLAGS                                                          \
    (__WASI_FDFLAG_RSYNC | __WASI_FDFLAG_APPEND | __WASI_FDFLAG_DSYNC |        \
     __WASI_FDFLAG_SYNC | __WASI_FDFLAG_NONBLOCK)

namespace storage {
std::string prependRuntimeRoot(const std::string& originalPath)
{
    conf::FaasmConfig& conf = conf::getFaasmConfig();
    boost::filesystem::path p(conf.runtimeFilesDir);
    p.append(originalPath);
    return p.string();
}

int32_t wasiFdFlagsToLinux(int32_t fdFlags)
{
    int32_t result = 0;

    // More open flags
    if (fdFlags != 0) {
        if (fdFlags & WASI_FD_FLAGS) {
            if (fdFlags & __WASI_FDFLAG_RSYNC) {
                result |= O_RSYNC;
            }

            if (fdFlags & __WASI_FDFLAG_APPEND) {
                result |= O_APPEND;
            }

            if (fdFlags & __WASI_FDFLAG_DSYNC) {
                result |= O_DSYNC;
            }

            if (fdFlags & __WASI_FDFLAG_SYNC) {
                result |= O_SYNC;
            }

            if (fdFlags & __WASI_FDFLAG_NONBLOCK) {
                result |= O_NONBLOCK;
            }

        } else {
            throw std::runtime_error("Unhandled fd flags");
        }
    }

    return result;
}

OpenMode getOpenMode(uint16_t openFlags)
{
    if (openFlags & __WASI_O_CREAT) {
        return OpenMode::CREATE;
    } else if (openFlags & __WASI_O_DIRECTORY) {
        return OpenMode::DIRECTORY;
    } else if (openFlags & __WASI_O_TRUNC) {
        return OpenMode::TRUNC;
    } else if (openFlags & __WASI_O_EXCL) {
        return OpenMode::EXCL;
    } else if (openFlags == 0) {
        return OpenMode::NONE;
    } else {
        throw std::runtime_error("Unable to detect open mode");
    }
}

ReadWriteType getRwType(uint64_t rights)
{
    // Check for presence of any read/write permissions
    const bool rightsRead = rights & WASI_RIGHTS_READ;
    const bool rightsWrite = rights & WASI_RIGHTS_WRITE;

    if (rightsRead && rightsWrite) {
        return ReadWriteType::READ_WRITE;
    } else if (rightsRead) {
        return ReadWriteType::READ_ONLY;
    } else if (!rightsRead && rightsWrite) {
        return ReadWriteType::WRITE_ONLY;
    } else if (rights == 0) {
        return ReadWriteType::NO_READ_WRITE;
    } else {
        return ReadWriteType::CUSTOM;
    }
}

uint16_t errnoToWasi(int errnoIn)
{
    switch (errnoIn) {
        case EPERM:
            return __WASI_EPERM;
        case ENOENT:
            return __WASI_ENOENT;
        case EIO:
            return __WASI_EIO;
        case EBADF:
            return __WASI_EBADF;
        case ENOMEM:
            return __WASI_ENOMEM;
        case EACCES:
            return __WASI_EACCES;
        case EEXIST:
            return __WASI_EEXIST;
        case ENOTDIR:
            return __WASI_ENOTDIR;
        case EISDIR:
            return __WASI_EISDIR;
        case EINVAL:
            return __WASI_EINVAL;
        case EMFILE:
            return __WASI_EMFILE;
        default:
            throw std::runtime_error("Unsupported WASI errno: " +
                                     std::to_string(errnoIn));
    }
}

std::string FileDescriptor::getPath()
{
    return path;
}

void FileDescriptor::setPath(const std::string& newPath)
{
    path = newPath;
}

std::string FileDescriptor::absPath(const std::string& relativePath)
{
    std::string res;

    if (relativePath.empty()) {
        return path;
    } else {
        std::string basePath = path == "." ? "" : path;
        boost::filesystem::path joinedPath(basePath);
        joinedPath.append(relativePath);
        return joinedPath.string();
    }
}

FileDescriptor FileDescriptor::stdFdFactory(int stdFd,
                                            const std::string& devPath)
{
    FileDescriptor fdStd;
    fdStd.setPath(devPath);

    uint64_t rights = __WASI_RIGHT_FD_READ | __WASI_RIGHT_FD_FDSTAT_SET_FLAGS |
                      __WASI_RIGHT_FD_WRITE | __WASI_RIGHT_FD_FILESTAT_GET |
                      __WASI_RIGHT_POLL_FD_READWRITE;
    fdStd.setActualRights(rights, rights);
    fdStd.linuxFd = stdFd;
    return fdStd;
}

FileDescriptor FileDescriptor::stdoutFactory()
{
    return FileDescriptor::stdFdFactory(STDOUT_FILENO, "/dev/stdout");
}

FileDescriptor FileDescriptor::stdinFactory()
{
    return FileDescriptor::stdFdFactory(STDIN_FILENO, "/dev/stdin");
}

FileDescriptor FileDescriptor::stderrFactory()
{
    return FileDescriptor::stdFdFactory(STDERR_FILENO, "/dev/stderr");
}

void FileDescriptor::iterReset()
{
    // Reset iterator state
    dirContentsLoaded = false;
    dirContentsIdx = 0;
    dirContents.clear();
}

void FileDescriptor::loadDirContents()
{
    auto logger = faabric::util::getLogger();

    // Work out the local filesystem path
    std::string realPath;
    if (SharedFiles::isPathShared(path)) {
        int pullErr = SharedFiles::syncSharedFile(path);

        if (pullErr != 0) {
            throw std::runtime_error("Failed to open shared dir");
        }

        realPath = SharedFiles::realPathForSharedFile(path);
    } else {
        realPath = prependRuntimeRoot(path);
    }

    // Open the directory
    logger->debug("Loading dir contents: {}", realPath);
    DIR* dirPtr = ::opendir(realPath.c_str());
    if (dirPtr == nullptr) {
        throw std::runtime_error("Failed to open dir");
    }

    // Load all directory entries
    uint64_t nextIdx = 0;
    struct dirent* direntPtr;
    while ((direntPtr = ::readdir(dirPtr)) != nullptr) {
        nextIdx++;

        DirEnt nextEnt;

        // We have to set "next" here to specify the offset of this
        // directory entry. It seems this is only used to be passed
        // back as the "cookie" value to fd_readdir
        nextEnt.next = nextIdx;

        nextEnt.type = direntPtr->d_type;
        nextEnt.ino = direntPtr->d_ino;
        nextEnt.path = std::string(direntPtr->d_name);

        dirContents.push_back(nextEnt);
    }

    // Close iterator
    closedir(dirPtr);
    logger->debug("Loaded {} entries for {}", dirContents.size(), realPath);

    // Set flag
    dirContentsLoaded = true;
}

void FileDescriptor::iterBack()
{
    if (!dirContentsLoaded) {
        throw std::runtime_error("Iterator not started, cannot go back");
    }

    if (dirContentsIdx == 0) {
        throw std::runtime_error("Iterator already at zero, cannot go back");
    }

    dirContentsIdx--;
}

bool FileDescriptor::iterStarted()
{
    return dirContentsLoaded;
}

bool FileDescriptor::iterFinished()
{
    return dirContentsLoaded && (dirContentsIdx >= dirContents.size());
}

DirEnt FileDescriptor::iterNext()
{
    // Check if contents are initialised
    if (!dirContentsLoaded) {
        loadDirContents();
    }

    // Check
    if (iterFinished()) {
        throw std::runtime_error(
          fmt::format("Accessing index {} in directory length {}",
                      dirContentsIdx,
                      dirContents.size()));
    }

    DirEnt nextEntry = dirContents.at(dirContentsIdx);

    // Increment the iterator
    dirContentsIdx++;

    return nextEntry;
}

/**
 * Note that this function conforms to the standard readdir interface:
 * - Copy each dirent struct followed by its path string
 * - If the last path string doesn't fit, return the buffer length
 * - If the last dirent doesn't fit, don't copy it and return the buffer length
 *
 * The external libc wrapper should prevent this third point.
 *
 * If the final dirent or path doesn't fit, the caller will expect the next
 * result to contain that same dirent/ path, so we have to step back one in the
 * iterator if that's the case.
 *
 * The caller knows they've reached the end when the buffer is _not_ filled by
 * this call.
 */
size_t FileDescriptor::copyDirentsToWasiBuffer(uint8_t* buffer,
                                               size_t bufferLen)
{
    auto logger = faabric::util::getLogger();

    // Prepare loop variables
    size_t bytesLeft = bufferLen;
    bool isBufferFull = false;

    size_t wasiDirentSize = sizeof(__wasi_dirent_t);

    while (!iterFinished()) {
        DirEnt d = iterNext();

        // Create WASI dirent
        size_t pathSize = d.path.size();
        __wasi_dirent_t wasmDirEnt{ .d_next = d.next,
                                    .d_ino = d.ino,
                                    .d_namlen = (uint32_t)pathSize,
                                    .d_type = d.type };

        // Drop out if we have no room for the dirent
        if (bytesLeft < wasiDirentSize) {
            isBufferFull = true;
            break;
        }

        auto direntPtr = BYTES(&wasmDirEnt);
        std::copy(direntPtr, direntPtr + wasiDirentSize, buffer);
        bytesLeft -= wasiDirentSize;
        buffer += wasiDirentSize;

        // Drop out if we have no room for the path
        if (bytesLeft < pathSize) {
            isBufferFull = true;
            break;
        }

        auto pathPtr = BYTES_CONST(d.path.c_str());
        std::copy(pathPtr, pathPtr + pathSize, buffer);
        bytesLeft -= pathSize;
        buffer += pathSize;
    }

    if (isBufferFull) {
        // Go back one, we've not been able to finish this entry
        iterBack();
        return bufferLen;
    } else {
        // Return the number of bytes copied
        return bufferLen - bytesLeft;
    }
}

bool FileDescriptor::pathOpen(uint32_t lookupFlags,
                              uint32_t openFlags,
                              int32_t fdFlags)
{
    if (!rightsSet) {
        throw std::runtime_error("Opening path with no rights set");
    }

    auto logger = faabric::util::getLogger();

    OpenMode openMode = getOpenMode((uint16_t)openFlags);

    ReadWriteType rwType = getRwType(actualRightsBase);

    int readWrite = 0;
    if (openMode == OpenMode::DIRECTORY) {
        readWrite = O_RDONLY;
    } else if (rwType == ReadWriteType::READ_WRITE) {
        readWrite = O_RDWR;
    } else if (rwType == ReadWriteType::WRITE_ONLY) {
        readWrite = O_WRONLY;
    } else if (rwType == ReadWriteType::READ_ONLY) {
        readWrite = O_RDONLY;
    } else if (rwType == ReadWriteType::NO_READ_WRITE) {
        readWrite = 0;
    } else {
        logger->error("Unrecognised access mode: {}", rwType);
        throw std::runtime_error("Unrecognised access mode for file");
    }

    // Open flags
    if (openMode == OpenMode::CREATE) {
        linuxFlags = readWrite | O_CREAT;
        linuxMode = S_IRWXU | S_IRGRP | S_IROTH;
    } else if (openMode == OpenMode::DIRECTORY) {
        // Note - we can only pass O_RDONLY with O_DIRECTORY
        linuxFlags = readWrite | O_DIRECTORY;
        linuxMode = 0;
    } else if (openMode == OpenMode::TRUNC) {
        linuxFlags = readWrite | O_TRUNC;
        linuxMode = 0;
    } else if (openMode == OpenMode::EXCL) {
        linuxFlags = readWrite | O_EXCL;
        linuxMode = 0;
    } else if (openMode == OpenMode::NONE) {
        linuxFlags = readWrite;
        linuxMode = 0;
    } else {
        logger->error("Unrecognised open mode: {}", openMode);
        throw std::runtime_error("Unrecognised open flags");
    }

    // More flags
    linuxFlags |= wasiFdFlagsToLinux(fdFlags);

    bool isShared = SharedFiles::isPathShared(path);
    std::string realPath;
    if (isShared) {
        // Pull the shared file
        linuxErrno = SharedFiles::syncSharedFile(path);
        if (linuxErrno != 0) {
            linuxFd = -1;
            wasiErrno = errnoToWasi(linuxErrno);
            return false;
        }

        realPath = SharedFiles::realPathForSharedFile(path);
    } else {
        realPath = prependRuntimeRoot(path);
    }

    // Attempt to open the local file
    if (realPath == "/dev/urandom") {
        // TODO avoid use of system-wide urandom
        linuxFd = ::open("/dev/urandom", 0, 0);

    } else if (realPath == "/dev/null") {
        linuxFd = ::open("/dev/null", 0, 0);
    } else {
        linuxFd = ::open(realPath.c_str(), linuxFlags, linuxMode);
    }

    if (linuxFd < 0) {
        linuxErrno = errno;
        wasiErrno = errnoToWasi(linuxErrno);
        return false;
    }

    return true;
}

bool FileDescriptor::mkdir(const std::string& dirPath)
{
    std::string fullPath = prependRuntimeRoot(dirPath);
    int res = ::mkdir(fullPath.c_str(), 0755);

    if (res < 0) {
        wasiErrno = errnoToWasi(errno);
        return false;
    }

    return true;
}

bool FileDescriptor::updateFlags(int32_t fdFlags)
{
    // Update underlying file descriptor
    int32_t newFlags = wasiFdFlagsToLinux(fdFlags);

    int res = fcntl(linuxFd, F_SETFL, newFlags);
    if (res < 0) {
        wasiErrno = errnoToWasi(errno);
        return false;
    }

    // Update existing Linux flags
    linuxFlags |= newFlags;
    return true;
}

void FileDescriptor::close()
{
    if (linuxFd > 0) {
        ::close(linuxFd);
    }
}

bool FileDescriptor::unlink(const std::string& relativePath)
{
    std::string fullPath = absPath(relativePath);
    const std::string maskedPath = prependRuntimeRoot(fullPath);
    int res = ::unlink(maskedPath.c_str());

    if (res != 0) {
        wasiErrno = errnoToWasi(errno);
        return false;
    }

    return true;
}

bool FileDescriptor::rmdir(const std::string& relativePath)
{
    std::string fullPath = absPath(relativePath);
    const std::string maskedPath = prependRuntimeRoot(fullPath);
    int res = ::rmdir(maskedPath.c_str());

    if (res != 0) {
        wasiErrno = errnoToWasi(errno);
        return false;
    }

    return true;
}

bool FileDescriptor::rename(const std::string& newPath,
                            const std::string& relativePath)
{
    std::string fullPath = absPath(relativePath);
    std::string fullOldPath = prependRuntimeRoot(fullPath);
    std::string fullNewPath = prependRuntimeRoot(newPath);

    int res = ::rename(fullOldPath.c_str(), fullNewPath.c_str());

    if (res != 0) {
        wasiErrno = errnoToWasi(-1 * res);
        return false;
    }

    return true;
}

Stat FileDescriptor::stat(const std::string& relativePath)
{
    struct stat nativeStat
    {};

    int statErrno = 0;
    if (linuxFd == STDOUT_FILENO || linuxFd == STDIN_FILENO ||
        linuxFd == STDERR_FILENO) {
        int result = ::fstat(linuxFd, &nativeStat);
        if (result < 0) {
            statErrno = errno;
        }
    } else {
        // Work out whether we're stat-ing a shared path
        std::string statPath = absPath(relativePath);
        std::string realPath;
        if (SharedFiles::isPathShared(statPath)) {
            statErrno = SharedFiles::syncSharedFile(statPath);
            if (statErrno == 0) {
                realPath = SharedFiles::realPathForSharedFile(statPath);
            }
        } else {
            realPath = prependRuntimeRoot(statPath);
        }

        // Do the actual stat
        int result;
        if (!realPath.empty()) {
            result = ::stat(realPath.c_str(), &nativeStat);
            if (result < 0) {
                statErrno = errno;
            }
        }
    }

    Stat statResult;
    if (statErrno != 0) {
        statResult.failed = true;
        statResult.wasiErrno = errnoToWasi(statErrno);
        return statResult;
    } else {
        statResult.failed = false;
        statResult.wasiErrno = 0;
    }

    // Work out file type
    bool isReadOnly = false;
    if (linuxFd == STDOUT_FILENO || linuxFd == STDIN_FILENO ||
        linuxFd == STDERR_FILENO) {
        statResult.wasiFiletype = __WASI_FILETYPE_CHARACTER_DEVICE;
    } else if (S_ISREG(nativeStat.st_mode)) {
        statResult.wasiFiletype = __WASI_FILETYPE_REGULAR_FILE;
    } else if (S_ISBLK(nativeStat.st_mode)) {
        statResult.wasiFiletype = __WASI_FILETYPE_BLOCK_DEVICE;
    } else if (S_ISDIR(nativeStat.st_mode)) {
        statResult.wasiFiletype = __WASI_FILETYPE_DIRECTORY;
        isReadOnly = true;
    } else if (S_ISLNK(nativeStat.st_mode)) {
        statResult.wasiFiletype = __WASI_FILETYPE_SYMBOLIC_LINK;
    } else if (S_ISCHR(nativeStat.st_mode)) {
        statResult.wasiFiletype = __WASI_FILETYPE_CHARACTER_DEVICE;
    } else {
        throw std::runtime_error("Unrecognised file type");
    }

    // Set up the result
    statResult.st_dev = nativeStat.st_dev;
    statResult.st_ino = nativeStat.st_ino;
    statResult.st_nlink = nativeStat.st_nlink;
    statResult.st_size = nativeStat.st_size;
    statResult.st_mode = nativeStat.st_mode;
    statResult.st_atim = faabric::util::timespecToNanos(&nativeStat.st_atim);
    statResult.st_mtim = faabric::util::timespecToNanos(&nativeStat.st_mtim);
    statResult.st_ctim = faabric::util::timespecToNanos(&nativeStat.st_ctim);

    return statResult;
}

ssize_t FileDescriptor::readLink(const std::string& relativePath,
                                 char* buffer,
                                 size_t bufferLen)
{
    std::string linkPath = prependRuntimeRoot(absPath(relativePath));

    if (SharedFiles::isPathShared(linkPath)) {
        faabric::util::getLogger()->error(
          "Readlink on shared not yet supported ({})", path);
        throw std::runtime_error("Readlink on shared file not supported");
    }

    ssize_t bytesRead = ::readlink(linkPath.c_str(), buffer, (size_t)bufferLen);
    return bytesRead;
}

uint16_t FileDescriptor::seek(int64_t offset,
                              int wasiWhence,
                              uint64_t* newOffset)
{
    int linuxWhence;
    if (wasiWhence == __WASI_WHENCE_SET) {
        linuxWhence = SEEK_SET;
    } else if (wasiWhence == __WASI_WHENCE_CUR) {
        linuxWhence = SEEK_CUR;
    } else if (wasiWhence == __WASI_WHENCE_END) {
        linuxWhence = SEEK_END;
    } else {
        throw std::runtime_error("Unsupported whence");
    }

    // Do the seek
    off_t result = ::lseek(linuxFd, offset, linuxWhence);
    if (result < 0) {
        return errnoToWasi(errno);
    }

    *newOffset = (uint64_t)result;

    return __WASI_ESUCCESS;
}

uint64_t FileDescriptor::tell()
{
    off_t result = ::lseek(linuxFd, 0, SEEK_CUR);
    return result;
}

int FileDescriptor::getLinuxFd()
{
    return linuxFd;
}

int FileDescriptor::getLinuxFlags()
{
    return linuxFlags;
}

int FileDescriptor::getLinuxErrno()
{
    return linuxErrno;
}

uint16_t FileDescriptor::getWasiErrno()
{
    return wasiErrno;
}

uint64_t FileDescriptor::getActualRightsBase()
{
    return actualRightsBase;
}

uint64_t FileDescriptor::getActualRightsInheriting()
{
    return actualRightsInheriting;
}

void FileDescriptor::setActualRights(uint64_t rights, uint64_t inheriting)
{
    actualRightsBase = rights;
    actualRightsInheriting = inheriting;
    rightsSet = true;
}

int FileDescriptor::duplicate(const FileDescriptor& other)
{
    // Duplicate the underlying fd
    linuxFd = ::dup(other.linuxFd);

    linuxMode = other.linuxMode;
    linuxFlags = other.linuxFlags;
    linuxErrno = other.linuxErrno;

    // Set the internal variables
    path = other.path;
    rightsSet = other.rightsSet;
    actualRightsBase = other.actualRightsBase;
    actualRightsInheriting = other.actualRightsInheriting;

    dirContentsLoaded = other.dirContentsLoaded;
    dirContents = other.dirContents;
    dirContentsIdx = other.dirContentsIdx;

    return linuxFd;
}

}
