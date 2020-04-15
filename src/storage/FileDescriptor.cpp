#include "FileDescriptor.h"
#include "syscall.h"

#include <util/timing.h>

#include <utility>
#include <dirent.h>
#include <stdexcept>
#include <boost/filesystem.hpp>
#include <WAVM/WASI/WASIABI.h>
#include <fcntl.h>
#include <util/strings.h>
#include <util/locks.h>
#include <util/config.h>
#include <util/logging.h>
#include <storage/FileLoader.h>
#include <util/files.h>
#include <sys/stat.h>


#define SHARED_FILE_PREFIX "faasm://"

#define WASI_FD_FLAGS (__WASI_FDFLAG_RSYNC | __WASI_FDFLAG_APPEND | __WASI_FDFLAG_DSYNC | __WASI_FDFLAG_SYNC | __WASI_FDFLAG_NONBLOCK)

namespace storage {
    enum FileState {
        NOT_CHECKED,
        NOT_EXISTS,
        EXISTS_DIR,
        EXISTS
    };

    static std::shared_mutex sharedFileMapMutex;
    static std::unordered_map<std::string, FileState> sharedFileMap;

    std::string prependRuntimeRoot(const std::string &originalPath) {
        util::SystemConfig &conf = util::getSystemConfig();
        boost::filesystem::path p(conf.runtimeFilesDir);
        p.append(originalPath);
        return p.string();
    }

    std::string prependSharedRoot(const std::string &originalPath) {
        util::SystemConfig &conf = util::getSystemConfig();
        boost::filesystem::path p(conf.sharedFilesDir);
        p.append(originalPath);
        return p.string();
    }

    std::string realPathForSharedFile(const std::string &sharedPath) {
        std::string strippedPath = util::removeSubstr(sharedPath, SHARED_FILE_PREFIX);
        std::string realPath = prependSharedRoot(sharedPath);
        return realPath;
    }

    bool FileDescriptor::isPathShared(const std::string &p) {
        return util::startsWith(p, SHARED_FILE_PREFIX);
    }

    OpenMode getOpenMode(uint16_t openFlags) {
        if (openFlags & __WASI_O_DIRECTORY) {
            return OpenMode::DIRECTORY;
        } else if (openFlags & __WASI_O_CREAT) {
            return OpenMode::CREATE;
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

    ReadWriteType getRwType(uint64_t rights) {
        // Work out read/write
        const bool rightsRead = rights & WASI_RIGHTS_READ;
        const bool rightsWrite = rights & WASI_RIGHTS_WRITE;

        if (rightsRead && rightsWrite) {
            return ReadWriteType::READ_WRITE;
        } else if (rightsRead) {
            return ReadWriteType::READ_ONLY;
        } else if (!rightsRead && rightsWrite) {
            return ReadWriteType::WRITE_ONLY;
        } else {
            return ReadWriteType::CUSTOM;
        }
    }

    uint16_t errnoToWasi(int errnoIn) {
        switch (errnoIn) {
            case EPERM:
                return __WASI_EPERM;
            case EBADF:
                return __WASI_EBADF;
            case EINVAL:
                return __WASI_EINVAL;
            case ENOENT:
                return __WASI_ENOENT;
            case EISDIR:
                return __WASI_EISDIR;
            default:
                throw std::runtime_error("Unsupported WASI errno");
        }
    }

    std::string FileDescriptor::getPath() {
        return path;
    }

    void FileDescriptor::setPath(const std::string &newPath) {
        path = newPath;
    }

    std::string FileDescriptor::absPath(const std::string &relativePath) {
        std::string res;

        if (relativePath.empty()) {
            res = path;
        } else {
            std::string basePath = path == "." ? "" : path;
            boost::filesystem::path joinedPath(basePath);
            joinedPath.append(relativePath);
            res = joinedPath.string();
        }

        return res;
    }

    FileDescriptor FileDescriptor::stdFdFactory(int stdFd, const std::string &devPath) {
        FileDescriptor fdStd;
        fdStd.setPath(devPath);

        uint64_t rights = __WASI_RIGHT_FD_READ | __WASI_RIGHT_FD_FDSTAT_SET_FLAGS
                          | __WASI_RIGHT_FD_WRITE | __WASI_RIGHT_FD_FILESTAT_GET
                          | __WASI_RIGHT_POLL_FD_READWRITE;
        fdStd.setActualRights(rights, rights);
        fdStd.linuxFd = stdFd;
        return fdStd;
    }

    FileDescriptor FileDescriptor::stdoutFactory() {
        return FileDescriptor::stdFdFactory(STDOUT_FILENO, "/dev/stdout");
    }

    FileDescriptor FileDescriptor::stdinFactory() {
        return FileDescriptor::stdFdFactory(STDIN_FILENO, "/dev/stdin");
    }

    FileDescriptor FileDescriptor::stderrFactory() {
        return FileDescriptor::stdFdFactory(STDERR_FILENO, "/dev/stderr");
    }

    FileDescriptor::FileDescriptor() : iterStarted(false), iterFinished(false),
                                       dirPtr(nullptr), direntPtr(nullptr),
                                       rightsSet(false),
                                       linuxFd(-1), linuxMode(-1), linuxFlags(-1), linuxErrno(0),
                                       wasiErrno(0) {

    }

    DirEnt FileDescriptor::iterNext() {
        if (iterFinished) {
            throw std::runtime_error("Directory iterator finished");
        }

        if (!iterStarted) {
            iterStarted = true;

            std::string realPath;
            dirPtr = nullptr;
            if (isPathShared(path)) {
                int pullErr = pullSharedFile(path);

                if (pullErr != 0) {
                    throw std::runtime_error("Failed to open shared dir");
                }

                realPath = realPathForSharedFile(path);
            } else {
                realPath = prependRuntimeRoot(path);
            }

            dirPtr = ::opendir(realPath.c_str());
            if (dirPtr == nullptr) {
                throw std::runtime_error("Failed to open dir");
            }
        }

        // Call readdir to get next dirent
        direntPtr = ::readdir(dirPtr);

        // Build the actual dirent
        DirEnt d;
        if (!direntPtr) {
            closedir(dirPtr);
            iterFinished = true;
            d.isEnd = true;
        } else {
            d.type = direntPtr->d_type;
            d.ino = direntPtr->d_ino;
            d.path = std::string(direntPtr->d_name);
        }

        return d;
    }

    bool
    FileDescriptor::pathOpen(uint32_t lookupFlags, uint32_t openFlags, int32_t fdFlags) {
        if (!rightsSet) {
            throw std::runtime_error("Opening path with no rights set");
        }

        OpenMode openMode = getOpenMode((uint16_t) openFlags);
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
        } else {
            // Default to r/w
            readWrite = O_RDWR;
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
            throw std::runtime_error("Unrecognised open flags");
        }

        // More open flags
        if (fdFlags != 0) {
            if (fdFlags & WASI_FD_FLAGS) {
                if (fdFlags & __WASI_FDFLAG_RSYNC) {
                    linuxFlags |= O_RSYNC;
                }

                if (fdFlags & __WASI_FDFLAG_APPEND) {
                    linuxFlags |= O_APPEND;
                }

                if (fdFlags & __WASI_FDFLAG_DSYNC) {
                    linuxFlags |= O_DSYNC;
                }

                if (fdFlags & __WASI_FDFLAG_SYNC) {
                    linuxFlags |= O_SYNC;
                }

                if (fdFlags & __WASI_FDFLAG_NONBLOCK) {
                    linuxFlags |= O_NONBLOCK;
                }

            } else {
                throw std::runtime_error("Unhandled fd flags");
            }
        }

        bool isShared = isPathShared(path);
        std::string realPath;
        if (isShared) {
            // Pull the shared file
            linuxErrno = pullSharedFile(path);
            if (linuxErrno != 0) {
                linuxFd = -1;
                wasiErrno = errnoToWasi(linuxErrno);
                return false;
            }

            realPath = realPathForSharedFile(path);
        } else {
            realPath = prependRuntimeRoot(path);;
        }

        // Attempt to open the local file
        if (realPath == "/dev/urandom") {
            //TODO avoid use of system-wide urandom
            linuxFd = open("/dev/urandom", 0, 0);

        } else if (realPath == "/dev/null") {
            linuxFd = open("/dev/null", 0, 0);
        } else {
            linuxFd = open(realPath.c_str(), linuxFlags, linuxMode);
        }

        if (linuxFd < 0) {
            linuxErrno = errno;
            wasiErrno = errnoToWasi(linuxErrno);
            return false;
        }

        return true;
    }

    bool FileDescriptor::mkdir(const std::string &dirPath) {
        std::string fullPath = prependRuntimeRoot(path);
        int res = ::mkdir(fullPath.c_str(), 0755);

        if (res < 0) {
            wasiErrno = errnoToWasi(errno);
            return false;
        }

        return true;
    }

    void FileDescriptor::close() {

    }

    bool FileDescriptor::unlink(const std::string &relativePath) {
        std::string fullPath = absPath(relativePath);
        const std::string maskedPath = prependRuntimeRoot(fullPath);
        int res = ::unlink(maskedPath.c_str());

        if (res != 0) {
            wasiErrno = errnoToWasi(errno);
            return false;
        }

        return true;
    }

    bool FileDescriptor::rename(const std::string &newPath, const std::string &relativePath) {
        std::string fullPath = absPath(relativePath);
        std::string fullOldPath = prependRuntimeRoot(fullPath);
        int res = ::rename(fullOldPath.c_str(), newPath.c_str());

        if (res != 0) {
            wasiErrno = errnoToWasi(-1 * res);
            return false;
        }

        return true;
    }

    Stat FileDescriptor::stat(const std::string &relativePath) {
        struct stat nativeStat{};

        int statErrno = 0;
        if (linuxFd == STDOUT_FILENO || linuxFd == STDIN_FILENO || linuxFd == STDERR_FILENO) {
            int result = ::fstat(linuxFd, &nativeStat);
            if (result < 0) {
                statErrno = errno;
            }
        } else {
            std::string statPath = absPath(relativePath);

            int result;
            std::string realPath;
            if (isPathShared(statPath)) {
                statErrno = pullSharedFile(statPath);
                if (statErrno == 0) {
                    realPath = realPathForSharedFile(statPath);
                }
            } else {
                realPath = prependRuntimeRoot(path);
            }

            // Do the actual stat
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
        }

        // Work out file type
        bool isReadOnly = false;
        if (linuxFd == STDOUT_FILENO || linuxFd == STDIN_FILENO || linuxFd == STDERR_FILENO) {
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

        // Set WASI rights accordingly
        if (isReadOnly) {
            setActualRights(WASI_RIGHTS_READ, WASI_RIGHTS_READ);
        } else {
            uint64_t rights = WASI_RIGHTS_READ | WASI_RIGHTS_WRITE;
            setActualRights(rights, rights);
        }

        // Set up the result
        statResult.st_dev = nativeStat.st_dev;
        statResult.st_ino = nativeStat.st_ino;
        statResult.st_nlink = nativeStat.st_nlink;
        statResult.st_size = nativeStat.st_size;
        statResult.st_mode = nativeStat.st_mode;
        statResult.st_atim = util::timespecToNanos(&nativeStat.st_atim);
        statResult.st_mtim = util::timespecToNanos(&nativeStat.st_mtim);
        statResult.st_ctim = util::timespecToNanos(&nativeStat.st_ctim);

        return statResult;
    }

    ssize_t FileDescriptor::readLink(const std::string &relativePath, char *buffer, size_t bufferLen) {
        std::string linkPath = prependRuntimeRoot(absPath(relativePath));

        if (isPathShared(linkPath)) {
            util::getLogger()->error("Readlink on shared not yet supported ({})", path);
            throw std::runtime_error("Readlink on shared file not supported");
        }

        ssize_t bytesRead = ::readlink(linkPath.c_str(), buffer, (size_t) bufferLen);
        return bytesRead;
    }

    uint16_t FileDescriptor::seek(uint64_t offset, int linuxWhence, uint64_t *newOffset) {
        // Do the seek
        off_t result = ::lseek(linuxFd, offset, linuxWhence);
        if (result < 0) {
            return errnoToWasi(errno);
        }

        *newOffset = (uint64_t) result;

        return __WASI_ESUCCESS;
    }

    uint64_t FileDescriptor::tell() {
        off_t result = ::lseek(linuxFd, 0, SEEK_CUR);
        return result;
    }

    int FileDescriptor::getLinuxFd() {
        return linuxFd;
    }

    int FileDescriptor::getLinuxErrno() {
        return linuxErrno;
    }

    uint16_t FileDescriptor::getWasiErrno() {
        return wasiErrno;
    }

    uint64_t FileDescriptor::getActualRightsBase() {
        return actualRightsBase;
    }

    uint64_t FileDescriptor::getActualRightsInheriting() {
        return actualRightsInheriting;
    }

    void FileDescriptor::setActualRights(uint64_t rights, uint64_t inheriting) {
        actualRightsBase = rights;
        actualRightsInheriting = inheriting;
        rightsSet = true;
    }

    int FileDescriptor::pullSharedFile(const std::string &sharedPath) {
        // Pull if need be
        if (sharedFileMap.count(sharedPath) == 0) {
            std::string strippedPath = util::removeSubstr(sharedPath, SHARED_FILE_PREFIX);
            std::string realPath = prependSharedRoot(sharedPath);

            util::FullLock fullLock(sharedFileMapMutex);

            if (sharedFileMap.count(sharedPath) != 0) {
                return sharedFileMap[sharedPath];
            }

            sharedFileMap[sharedPath] = NOT_CHECKED;

            if (boost::filesystem::exists(realPath)) {
                // If already exists on filesystem, just mark it as such
                if (boost::filesystem::is_directory(realPath)) {
                    sharedFileMap[sharedPath] = EXISTS_DIR;
                } else {
                    sharedFileMap[sharedPath] = EXISTS;
                }
            } else {
                boost::filesystem::path p(realPath);

                FileLoader &loader = getFileLoader();
                std::vector<uint8_t> bytes;
                bool isDir = false;

                try {
                    bytes = loader.loadSharedFile(strippedPath);
                } catch (storage::SharedFileIsDirectoryException &e) {
                    isDir = true;
                }

                // Handle directories and files accordingly
                if (isDir) {
                    // Create directory if path is a directory
                    boost::filesystem::create_directories(p);
                    sharedFileMap[sharedPath] = EXISTS_DIR;
                } else if (bytes.empty()) {
                    sharedFileMap[sharedPath] = NOT_EXISTS;
                } else {
                    // Create parent directory
                    if (p.has_parent_path()) {
                        boost::filesystem::create_directories(p.parent_path());
                    }

                    // Write to file
                    util::writeBytesToFile(realPath, bytes);
                    sharedFileMap[sharedPath] = EXISTS;
                }
            }
        }

        FileState &state = sharedFileMap[sharedPath];
        switch (state) {
            case (NOT_EXISTS): {
                return ENOENT;
            }
            case (EXISTS_DIR):
            case (EXISTS): {
                return 0;
            }
            default: {
                return ENOENT;
            }
        }
    }
}
