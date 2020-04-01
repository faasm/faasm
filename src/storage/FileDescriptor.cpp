#include "FileDescriptor.h"

#include <utility>
#include <dirent.h>
#include <stdexcept>
#include <storage/SharedFilesManager.h>

#include <WAVM/WASI/WASIABI.h>
#include <fcntl.h>


namespace storage {
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

    FileDescriptor FileDescriptor::stdFdFactory(int stdFd, const std::string &devPath) {
        FileDescriptor fdStd(devPath);
        fdStd.rightsBase = __WASI_RIGHT_FD_READ | __WASI_RIGHT_FD_FDSTAT_SET_FLAGS
                           | __WASI_RIGHT_FD_WRITE | __WASI_RIGHT_FD_FILESTAT_GET
                           | __WASI_RIGHT_POLL_FD_READWRITE;

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


    FileDescriptor::FileDescriptor(std::string pathIn) : path(std::move(pathIn)),
                                                         iterStarted(false), iterFinished(false),
                                                         dirPtr(nullptr), direntPtr(nullptr),
                                                         linuxFd(-1), linuxMode(-1), linuxFlags(-1) {

    }

    DirEnt FileDescriptor::iterNext() {
        if (iterFinished) {
            throw std::runtime_error("Directory iterator finished");
        }

        if (!iterStarted) {
            iterStarted = true;

            storage::SharedFilesManager &sfm = storage::getSharedFilesManager();
            dirPtr = sfm.openDir(path);
            if (dirPtr == nullptr) {
                throw std::runtime_error("Failed to open dir");
            }
        }

        // Call readdir to get next dirent
        direntPtr = readdir(dirPtr);

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

    void FileDescriptor::path_open(uint64_t rightsBaseIn, uint64_t rightsInheritingIn, uint32_t openFlags) {
        rightsBase = rightsBaseIn;
        rightsInheriting = rightsInheritingIn;

        // Hacked from WAVM's WASIFile to ensure things have enough permissions not to break.
        const bool readDir = rightsBase & __WASI_RIGHT_FD_READDIR;
        const bool readFile = rightsBase & __WASI_RIGHT_FD_READDIR;

        const bool write = rightsBase & (__WASI_RIGHT_FD_DATASYNC | __WASI_RIGHT_FD_WRITE
                                         | __WASI_RIGHT_FD_ALLOCATE | __WASI_RIGHT_FD_FILESTAT_SET_SIZE);

        unsigned int osReadWrite = 0;
        if (readFile && write) {
            osReadWrite = O_RDWR;
        } else if (readFile || readDir) {
            osReadWrite = O_RDONLY;
        } else if (write && !readFile) {
            osReadWrite = O_WRONLY;
        } else {
            throw std::runtime_error("Unable to detect valid file flags");
        }

        uint16_t openFlagsCast = (uint16_t) openFlags;
        unsigned int osExtra = 0;
        if (openFlagsCast & (__WASI_O_CREAT | __WASI_O_DIRECTORY)) {
            osExtra = O_CREAT | O_DIRECTORY;
        } else if (openFlagsCast & (__WASI_O_CREAT | __WASI_O_TRUNC)) {
            osExtra = O_CREAT | O_TRUNC;
        } else if (openFlagsCast & (__WASI_O_CREAT | __WASI_O_EXCL)) {
            osExtra = O_CREAT | O_EXCL;
        } else if (openFlagsCast != 0) {
            throw std::runtime_error("Unrecognised flags on opening file");
        }

        linuxFlags = osReadWrite | osExtra;

        linuxMode = 0;
        if (openFlagsCast & __WASI_O_CREAT) {
            // Set create mode
            linuxMode = S_IRWXU | S_IRGRP | S_IROTH;
        }

        storage::SharedFilesManager &sfm = storage::getSharedFilesManager();
        linuxFd = sfm.openFile(path, linuxFlags, linuxMode);

//        // TODO - remove the negation fiddles here.
//        if (linuxFd < 0) {
//            // Our "open" returns a negative errno and callers in
//            // turn expect a negative errno.
//            return -1 * errnoToWasi(-1 * linuxFd);
//        } else {
//            return linuxFd;
//        }
    }

    void FileDescriptor::close() {

    }

    Stat FileDescriptor::stat() {
        struct stat64 nativeStat{};

        int result;
        if (linuxFd == STDOUT_FILENO) {
            result = ::fstat64(STDOUT_FILENO, &nativeStat);
        } else {
            storage::SharedFilesManager &sfm = storage::getSharedFilesManager();
            result = sfm.statFile(path, &nativeStat);
        }

        Stat s;
        if (result < 0) {
            s.failed = true;
            s.wasiErrno = errnoToWasi(-1 * result);
        }
        if (result == -EPERM) {
            s.failed = true;
            s.wasiErrno = __WASI_EPERM;
        } else if (result == -ENOENT) {
            s.failed = true;
            s.wasiErrno = __WASI_ENOENT;
        } else if (result < 0) {
            throw std::runtime_error("Unsupported error condition");
        }

        if (linuxFd == STDOUT_FILENO) {
            // Do nothing for stdout
        } else if (S_ISREG(nativeStat.st_mode)) {
            s.wasiFiletype = __WASI_FILETYPE_REGULAR_FILE;
        } else if (S_ISBLK(nativeStat.st_mode)) {
            s.wasiFiletype = __WASI_FILETYPE_BLOCK_DEVICE;
        } else if (S_ISDIR(nativeStat.st_mode)) {
            s.wasiFiletype = __WASI_FILETYPE_DIRECTORY;
        } else if (S_ISLNK(nativeStat.st_mode)) {
            s.wasiFiletype = __WASI_FILETYPE_SYMBOLIC_LINK;
        } else if (S_ISCHR(nativeStat.st_mode)) {
            s.wasiFiletype = __WASI_FILETYPE_CHARACTER_DEVICE;
        } else {
            throw std::runtime_error("Unrecognised file type");
        }

        return s;
    }

    uint16_t FileDescriptor::seek(uint64_t offset, int whence, uint64_t *newOffset) {
        int linuxWhence;
        if (whence == __WASI_WHENCE_CUR) {
            linuxWhence = SEEK_CUR;
        } else if (whence == __WASI_WHENCE_END) {
            linuxWhence = SEEK_END;
        } else if (whence == __WASI_WHENCE_SET) {
            linuxWhence = SEEK_SET;
        } else {
            throw std::runtime_error("Unsupported whence");
        }

        // Do the seek
        off_t result = ::lseek(linuxFd, offset, linuxWhence);
        if (result < 0) {
            return errnoToWasi(errno);
        }

        *newOffset = (uint64_t) result;

        return __WASI_ESUCCESS;
    }

    int FileDescriptor::getLinuxFd() {
        return linuxFd;
    }

    void FileDescriptor::setLinuxFd(int linuxFdIn) {
        linuxFd = linuxFdIn;
    }
}
