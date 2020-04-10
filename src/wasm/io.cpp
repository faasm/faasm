#include "WasmModule.h"
#include "syscalls.h"

#include <util/bytes.h>
#include <util/config.h>

#include <storage/FileDescriptor.h>

#include <dirent.h>
#include <poll.h>
#include <sys/uio.h>
#include <sys/ioctl.h>
#include <sys/unistd.h>
#include <sys/mman.h>

#include <WAVM/Runtime/Runtime.h>
#include <WAVM/Runtime/Intrinsics.h>
#include <WAVM/WASI/WASIABI.h>
#include <storage/FileLoader.h>
#include <storage/SharedFilesManager.h>
#include <util/macros.h>

/**
 * WASI filesystem handling
 *
 * The main WASI repo contains a fair bit of info on WASI's filesystem handling.
 *
 * https://github.com/WebAssembly/WASI/blob/master/phases/snapshot/docs.md
 *
 * You can also look at the WASI API:
 * https://github.com/WebAssembly/wasi-libc/blob/master/libc-bottom-half/headers/public/wasi/api.h
 */

namespace wasm {

    WAVM_DEFINE_INTRINSIC_FUNCTION(wasi, "fd_prestat_get", I32, wasi_fd_prestat_get, I32 fd, I32 prestatPtr) {
        util::getLogger()->debug("S - fd_prestat_get - {} {}", fd, prestatPtr);

        WasmModule *module = getExecutingModule();
        if (!module->getFileSystem().fileDescriptorExists(fd)) {
            return __WASI_EBADF;
        }

        storage::FileDescriptor &fileDesc = module->getFileSystem().getFileDescriptor(fd);

        auto wasiPrestat = &Runtime::memoryRef<__wasi_prestat_t>(module->defaultMemory, prestatPtr);
        wasiPrestat->pr_type = fileDesc.wasiPreopenType;
        wasiPrestat->u.dir.pr_name_len = fileDesc.path.size();

        return __WASI_ESUCCESS;
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(wasi, "fd_prestat_dir_name", I32, wasi_fd_prestat_dir_name, I32 fd, I32 pathPtr,
                                   I32 pathLen) {
        util::getLogger()->debug("S - fd_prestat_dir_name - {} {}", fd, pathPtr, pathLen);

        WasmModule *module = getExecutingModule();
        if (!module->getFileSystem().fileDescriptorExists(fd)) {
            return __WASI_EBADF;
        }

        storage::FileDescriptor &fileDesc = module->getFileSystem().getFileDescriptor(fd);

        // Copy the path into the wasm buffer
        char *buffer = Runtime::memoryArrayPtr<char>(module->defaultMemory, pathPtr, pathLen);
        std::copy(fileDesc.path.begin(), fileDesc.path.end(), buffer);

        return __WASI_ESUCCESS;
    }

    I32 doOpen(I32 pathPtr, int flags, int mode) {
        const std::string path = getStringFromWasm(pathPtr);

        storage::SharedFilesManager &sfm = storage::getSharedFilesManager();
        int fd = sfm.openFile(path, flags, mode);

        // NOTE: virtual filesystem will return the negative errno associated
        // with any failed operations
        if (fd > 0) {
            WasmModule *module = getExecutingModule();
            module->addFdForThisThread(fd);
        }

        return fd;
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(wasi, "path_open", I32, wasi_path_open,
                                   I32 rootFd,
                                   I32 lookupFlags,
                                   I32 path,
                                   I32 pathLen,
                                   U32 openFlags,
                                   U64 rightsBase,
                                   U64 rightsInheriting,
                                   I32 fdFlags,
                                   I32 resFdPtr) {

        const std::string pathStr = getStringFromWasm(path);
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->debug("S - path_open - {} {} {}", rootFd, pathStr, pathLen);

        // Open a new file descriptor
        // Returns a negative wasi errno if fails
        int fdRes = getExecutingModule()->getFileSystem().openFileDescriptor(
                rootFd, pathStr, rightsBase, rightsInheriting, lookupFlags, openFlags, fdFlags
        );

        if (fdRes < 0) {
            return -1 * fdRes;
        } else {
            // Write result to memory location
            Runtime::memoryRef<int>(getExecutingModule()->defaultMemory, resFdPtr) = fdRes;
            return __WASI_ESUCCESS;
        }
    }

    I32 s__open(I32 pathPtr, I32 flags, I32 mode) {
        util::getLogger()->debug("S - open - {} {} {}", pathPtr, flags, mode);
        return doOpen(pathPtr, flags, mode);
    }

    I32 s__dup(I32 oldFd) {
        util::getLogger()->debug("S - dup - {}", oldFd);

        WasmModule *module = getExecutingModule();

        module->checkThreadOwnsFd(oldFd);

        int newFd = dup(oldFd);
        module->addFdForThisThread(newFd);

        return newFd;
    }


    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "dup", I32, dup, I32 fd) {
        util::getLogger()->debug("S - dup - {}", fd);

        storage::FileSystem &fs = getExecutingModule()->getFileSystem();
        int newFd = fs.dup(fd);

        return newFd;
    }
    
    I32 s__fcntl64(I32 fd, I32 cmd, I32 c) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->debug("S - fcntl64 - {} {} {}", fd, cmd, c);

        WasmModule *module = getExecutingModule();

        module->checkThreadOwnsFd(fd);

        // Duplicating file descriptors
        int returnValue;
        if (cmd == F_DUPFD || cmd == F_DUPFD_CLOEXEC) {
            // Duplicating file descriptors is allowed
            returnValue = fcntl(fd, cmd, c);

            if (returnValue > 0) {
                module->addFdForThisThread(returnValue);
            }
        } else if (cmd == F_GETFL || cmd == F_GETFD || cmd == F_SETFD) {
            // Getting file flags, file descriptor flags and setting file descriptor flags are allowed
            returnValue = fcntl(fd, cmd, c);
        } else {
            logger->warn("Allowing arbitrary fcntl command: {}", cmd);
            returnValue = fcntl(fd, cmd, c);
        }

        if (returnValue < 0) {
            logger->warn("Failed fcntl call");
            return -errno;
        }

        return returnValue;
    }

    /**
     * This works a little like the normal Linux readdir, in that it will be called
     * repeatedly to get the full listing of a directory.
     *
     * The function should fill the read buffer until it's reached the final "page"
     * of results, at which point the returned size will be smaller than the read buffer.
     */
    WAVM_DEFINE_INTRINSIC_FUNCTION(wasi, "fd_readdir", I32, wasi_fd_readdir,
                                   I32 fd,
                                   I32 buf,
                                   I32 bufLen,
                                   U64 startCookie,
                                   I32 resSizePtr
    ) {
        util::getLogger()->debug("S - fd_readdir - {} {} {} {} {}", fd, buf, bufLen, startCookie, resSizePtr);

        storage::FileDescriptor &fileDesc = getExecutingModule()->getFileSystem().getFileDescriptor(fd);
        bool isStartCookie = startCookie == __WASI_DIRCOOKIE_START;

        if (fileDesc.iterStarted && isStartCookie) {
            throw std::runtime_error("Directory iterator already exists, but this is the start cookie");
        } else if (!fileDesc.iterStarted && !isStartCookie) {
            throw std::runtime_error("No directory iterator exists, and this is not the start cookie");
        }

        U8 *buffer = Runtime::memoryArrayPtr<U8>(getExecutingModule()->defaultMemory, buf, bufLen);
        size_t bytesCopied = 0;
        size_t bytesLeft = bufLen;

        if (!fileDesc.iterFinished) {
            while (bytesLeft > 0) {
                storage::DirEnt dirEnt = fileDesc.iterNext();

                // Done
                if (dirEnt.isEnd) {
                    break;
                }

                __wasi_dirent_t wasmDirEnt{
                        .d_next = dirEnt.next,
                        .d_type = dirEnt.type,
                        .d_ino = dirEnt.ino,
                        .d_namlen = (unsigned int) dirEnt.path.size()
                };

                // Copy the dirent itself
                int direntCopySize = util::safeCopyToBuffer(
                        BYTES(&wasmDirEnt), sizeof(wasmDirEnt),
                        buffer + bytesCopied, bytesLeft
                );
                bytesCopied += direntCopySize;
                bytesLeft -= direntCopySize;

                // Copy its name in straight after
                int pathCopySize = util::safeCopyToBuffer(
                        BYTES_CONST(dirEnt.path.c_str()), dirEnt.path.size(),
                        buffer + bytesCopied, bytesLeft
                );
                bytesCopied += pathCopySize;
                bytesLeft -= pathCopySize;
            }
        }

        // Set the result
        Runtime::memoryRef<U32>(getExecutingModule()->defaultMemory, resSizePtr) = bytesCopied;

        return 0;
    }

    /**
     * This function is tricky to implement as it's iterating through an unknown number of
     * files in the directory. We will be running two loops, one to fill up the given buffer,
     * and a nested loop to iterate through the native directory listing.
     *
     * We try to be conservative but will throw an exception if things aren't right. A bug here
     * can be hard to find.
     *
     * The musl implementation of readdir seems to require returning (-1 * errno) on error, not -1
     * as the man pages suggest.
     */
    I32 s__getdents64(I32 fd, I32 wasmDirentBuf, I32 wasmDirentBufLen) {
        util::getLogger()->debug("S - getdents64 - {} {} {}", fd, wasmDirentBuf, wasmDirentBufLen);

        Runtime::Memory *memoryPtr = getExecutingModule()->defaultMemory;
        U8 *hostWasmDirentBuf = &Runtime::memoryRef<U8>(memoryPtr, (Uptr) wasmDirentBuf);

        // Work out how many dirents we can fit (rounding down)
        size_t wasmDirentSize = sizeof(wasm_dirent64);

        // Create a small native buffer (can't overshoot the wasm offset)
        // Note that this can cause an EINVAL error if too small for the result
        int nativeBufLen = 80;

        U32 wasmBytesRead = 0;
        int wasmDirentCount = 0;

        // Here we will iterate getting native dirents until we've filled up the wasm buffer supplied
        for (wasmBytesRead = 0; wasmBytesRead < wasmDirentBufLen - (2 * wasmDirentSize);) {
            // Make the native syscall. This will read in a list of dirent structs to the buffer
            // We need to read at most two native dirents.
            auto nativeBuf = new char[nativeBufLen];
            long nativeBytesRead = syscall(SYS_getdents64, fd, nativeBuf, nativeBufLen);

            if (nativeBytesRead < 0) {
                // Error reading native dirents
                int newErrno = errno;
                delete[] nativeBuf;
                return -newErrno;
            } else if (nativeBytesRead == 0) {
                // End of directory
                delete[] nativeBuf;
                return wasmBytesRead;
            } else {
                // Now we iterate through the dirents we just got back from the host
                unsigned int nativeOffset;
                for (nativeOffset = 0; nativeOffset < nativeBytesRead;) {
                    // If we're going to overshoot on the wasm buffer, we have a problem (worth throwing an exception)
                    if (wasmBytesRead > (U32) wasmDirentBufLen) {
                        throw std::runtime_error("Overshot the end of the dirent buffer");
                    }

                    // Get a pointer to the native dirent
                    auto d = (struct dirent64 *) (nativeBuf + nativeOffset);

                    // Copy the relevant info into the wasm dirent.
                    struct wasm_dirent64 dWasm{};
                    dWasm.d_ino = (U32) d->d_ino;
                    dWasm.d_type = d->d_type;

                    // Copy the name into place
                    size_t nameLen = strlen(d->d_name);
                    std::copy(d->d_name, d->d_name + nameLen, dWasm.d_name);

                    // Copy the wasm dirent into place in wasm memory
                    auto dWasmBytes = BYTES(&dWasm);
                    std::copy(dWasmBytes, dWasmBytes + wasmDirentSize, hostWasmDirentBuf + wasmBytesRead);

                    // Move offsets along
                    nativeOffset += d->d_reclen;

                    wasmBytesRead += wasmDirentSize;
                    wasmDirentCount++;
                }
            }

            delete[] nativeBuf;
        }

        return wasmBytesRead;
    }

    /**
     * Read is ok provided the function owns the file descriptor
     */
    I32 s__read(I32 fd, I32 bufPtr, I32 bufLen) {
        util::getLogger()->debug("S - read - {} {} {}", fd, bufPtr, bufLen);

        // Provided the thread owns the fd, we allow reading.
        WasmModule *module = getExecutingModule();
        module->checkThreadOwnsFd(fd);

        // Get the buffer etc.
        Runtime::Memory *memoryPtr = getExecutingModule()->defaultMemory;
        U8 *buf = Runtime::memoryArrayPtr<U8>(memoryPtr, (Uptr) bufPtr, (Uptr) bufLen);

        // Do the actual read
        ssize_t bytesRead = read(fd, buf, (size_t) bufLen);

        return (I32) bytesRead;
    }

    I32 s__close(I32 fd) {
        util::getLogger()->debug("S - close - {}", fd);

        // Provided the thread owns the fd, we allow closing.
        WasmModule *module = getExecutingModule();
        module->checkThreadOwnsFd(fd);
        module->removeFdForThisThread(fd);

        close(fd);

        return 0;
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(wasi, "fd_close", I32, wasi_fd_close, I32 fd) {
        util::getLogger()->debug("S - fd_close - {}", fd);

        storage::FileDescriptor &fileDesc = getExecutingModule()->getFileSystem().getFileDescriptor(fd);
        fileDesc.close();

        return 0;
    }

    /** Poll is ok but can pass in an array of structs. */
    I32 s__poll(I32 fdsPtr, I32 nfds, I32 timeout) {
        util::getLogger()->debug("S - poll - {} {} {}", fdsPtr, nfds, timeout);

        auto *fds = Runtime::memoryArrayPtr<pollfd>(getExecutingModule()->defaultMemory, (Uptr) fdsPtr, (Uptr) nfds);
        WasmModule *module = getExecutingModule();

        // Check this thread has permission to poll the relevant fds
        for (int i = 0; i < nfds; i++) {
            pollfd fd = fds[i];
            module->checkThreadOwnsFd(fd.fd);
        }

        int pollRes = poll(fds, (size_t) nfds, timeout);
        return pollRes;
    }

    /**
     * Note that ioctl gets called in different ways. If it's called through the top-level
     * ioctl() function (as defined in ioctl.c), argsPtr will be a varargs array.
     *
     * Other places will call ioctl directly with syscall(SYS_ioctl, ...), in this case the
     * arguments may be direct pointers to arguments.
     *
     * We need to be careful which one we assume (you can check by inspecting the request flag,
     * and looking into the musl source).
     */
    I32 s__ioctl(I32 fd, I32 request, I32 argsPtr, I32 d, I32 e, I32 f) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->debug("S - ioctl - {} {} {} {} {} {}", fd, request, argsPtr, d, e, f);

        // Check ownership of fd
        getExecutingModule()->checkThreadOwnsFd(fd);

        if (request == TIOCGWINSZ) {
            // Getting the window size, importantly we assume this is call from isatty
            // where the argsPtr is a direct pointer to the winsize struct.
            auto ws = &Runtime::memoryRef<wasm_winsize>(getExecutingModule()->defaultMemory, argsPtr);
            ws->ws_col = 10;
            ws->ws_row = 11;
            ws->ws_xpixel = 100;
            ws->ws_ypixel = 101;

        } else {
            logger->warn("Unknown ioctl request: {}", request);
            throw std::runtime_error("Unknown ioctl request");
        }

        return 0;
    }

    ssize_t doWritev(int fd, iovec *nativeIovecs, I32 iovecCount) {
        ssize_t bytesWritten = writev(fd, nativeIovecs, iovecCount);

        // Catpure stdout if necessary, otherwise write as normal
        util::SystemConfig &conf = util::getSystemConfig();
        if (fd == STDOUT_FILENO && conf.captureStdout == "on") {
            getExecutingModule()->captureStdout(nativeIovecs, iovecCount);
        }

        delete[] nativeIovecs;

        return bytesWritten;
    }

    /**
     * Writing is ok provided the function owns the file descriptor
     */
    I32 s__writev(I32 fd, I32 iov, I32 iovcnt) {
        util::getLogger()->debug("S - writev - {} {} {}", fd, iov, iovcnt);

        WasmModule *module = getExecutingModule();
        module->checkThreadOwnsFd(fd);

        iovec *nativeIovecs = wasmIovecsToNativeIovecs(iov, iovcnt);
        return (I32) doWritev(fd, nativeIovecs, iovcnt);
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(wasi, "fd_write", I32, wasi_fd_write, I32 fd, I32 iovecsPtr, I32 iovecCount,
                                   I32 resBytesWrittenPtr) {
        util::getLogger()->debug("S - fd_write - {} {} {} {}", fd, iovecsPtr, iovecCount, resBytesWrittenPtr);

        storage::FileDescriptor &fileDesc = getExecutingModule()->getFileSystem().getFileDescriptor(fd);

        iovec *nativeIovecs = wasiIovecsToNativeIovecs(iovecsPtr, iovecCount);
        ssize_t bytesWritten = doWritev(fileDesc.getLinuxFd(), nativeIovecs, iovecCount);

        Runtime::memoryRef<int>(getExecutingModule()->defaultMemory, resBytesWrittenPtr) = (int) bytesWritten;

        return __WASI_ESUCCESS;
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(wasi, "fd_read", I32, wasi_fd_read, I32 fd, I32 iovecsPtr, I32 iovecCount,
                                   I32 resBytesRead) {
        util::getLogger()->debug("S - fd_read - {} {} {}", fd, iovecsPtr, iovecCount);
        storage::FileDescriptor &fileDesc = getExecutingModule()->getFileSystem().getFileDescriptor(fd);
        iovec *nativeIovecs = wasiIovecsToNativeIovecs(iovecsPtr, iovecCount);

        int bytesRead = readv(fileDesc.getLinuxFd(), nativeIovecs, iovecCount);
        Runtime::memoryRef<int>(getExecutingModule()->defaultMemory, resBytesRead) = (int) bytesRead;

        return __WASI_ESUCCESS;
    }

    I32 s__readv(I32 fd, I32 iovecPtr, I32 iovecCount) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->debug("S - readv - {} {} {}", fd, iovecPtr, iovecCount);

        getExecutingModule()->checkThreadOwnsFd(fd);

        iovec *nativeIovecs = wasmIovecsToNativeIovecs(iovecPtr, iovecCount);

        int bytesRead = readv(fd, nativeIovecs, iovecCount);
        if (bytesRead == -1) {
            logger->error("Failed readv {} - {}", errno, strerror(errno));
            throw std::runtime_error("Failed readv");
        }

        delete[] nativeIovecs;

        return bytesRead;
    }

    /**
     * Writing is ok provided the function owns the file descriptor
     */
    I32 s__write(I32 fd, I32 bufPtr, I32 bufLen) {
        util::getLogger()->debug("S - write - {} {} {}", fd, bufPtr, bufLen);

        // Provided the thread owns the fd, we allow reading.
        WasmModule *module = getExecutingModule();
        module->checkThreadOwnsFd(fd);

        // Get the buffer etc.
        Runtime::Memory *memoryPtr = module->defaultMemory;
        U8 *buf = Runtime::memoryArrayPtr<U8>(memoryPtr, (Uptr) bufPtr, (Uptr) bufLen);

        // Do the actual read
        ssize_t bytesWritten = write(fd, buf, (size_t) bufLen);

        return bytesWritten;
    }

    I32 s__mkdir(I32 pathPtr, I32 mode) {
        const std::string fakePath = getMaskedPathFromWasm(pathPtr);

        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->debug("S - mkdir - {} {}", fakePath, mode);

        int res = mkdir(fakePath.c_str(), mode);
        if (res < 0) {
            logger->error("Failed to mkdir at {} - code {}", fakePath, res);
            throw std::runtime_error("Failed on mkdir");
        }

        return res;
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(wasi, "path_create_directory", I32, wasi_path_create_directory, I32 fd, I32 path,
                                   I32 pathLen) {
        const std::string &pathStr = getStringFromWasm(path);
        util::getLogger()->debug("S - path_create_directory - {} {}", fd, pathStr);

        storage::FileDescriptor &fileDesc = getExecutingModule()->getFileSystem().getFileDescriptor(fd);
        bool success = fileDesc.mkdir(pathStr);

        if(!success) {
            return fileDesc.getWasiErrno();
        }
        
        return __WASI_ESUCCESS;
    }


    I32 s__rename(I32 srcPtr, I32 destPtr) {
        const std::string srcPath = getMaskedPathFromWasm(srcPtr);
        const std::string destPath = getMaskedPathFromWasm(destPtr);

        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->debug("S - rename - {} {}", srcPath, destPath);

        int res = rename(srcPath.c_str(), destPath.c_str());
        if (res != 0) {
            if (res == -EPERM) {
                logger->error("Permission error renaming {} -> {}", srcPath, destPath);
            } else {
                logger->error("Failed renaming {} -> {} - code {}", srcPath, destPath, res);
            }

            throw std::runtime_error("Failed renaming file");
        }
        return res;
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(wasi, "path_rename", I32, wasi_path_rename, I32 fd,
            I32 oldPath, I32 oldPathLen, I32 newFd, I32 newPath, I32 newPathLen) {
        std::string oldPathStr = getStringFromWasm(oldPath);
        std::string newPathStr = getStringFromWasm(newPath);

        util::getLogger()->debug("S - path_rename - {} {} {} {}", fd, oldPathStr, newFd, newPathStr);

        WasmModule *module = getExecutingModule();
        storage::FileDescriptor &oldFileDesc = module->getFileSystem().getFileDescriptor(fd);
        storage::FileDescriptor &newFileDesc = module->getFileSystem().getFileDescriptor(newFd);

        const std::string &fullNewPath = newFileDesc.absPath(newPathStr);
        bool success = oldFileDesc.rename(fullNewPath);
        if(!success) {
            return oldFileDesc.getWasiErrno();
        }

        return __WASI_ESUCCESS;
    }


    WAVM_DEFINE_INTRINSIC_FUNCTION(wasi, "path_unlink_file", I32, wasi_path_unlink_file, I32 rootFd, I32 pathPtr,
                                   I32 pathLen) {
        util::getLogger()->debug("S - path_unlink_file - {} {}", rootFd, pathPtr);

        std::string pathStr = getStringFromWasm(pathPtr);
        storage::FileDescriptor &fileDesc = getExecutingModule()->getFileSystem().getFileDescriptor(rootFd);
        bool success = fileDesc.unlink(pathStr);

        if (!success) {
            return fileDesc.getWasiErrno();
        }

        return 0;
    }

    I32 s__unlink(I32 pathPtr) {
        const std::string fakePath = getMaskedPathFromWasm(pathPtr);

        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->debug("S - unlink {}", fakePath);

        int res = unlink(fakePath.c_str());
        if (res < 0) {
            logger->error("Failed to unlink at {} - code {}", fakePath, res);
            throw std::runtime_error("Failed on mkdir");
        }

        return res;
    }

    I32 s__access(I32 pathPtr, I32 mode) {
        const std::string path = getMaskedPathFromWasm(pathPtr);
        util::getLogger()->debug("S - access - {} {}", path, mode);

        return access(path.c_str(), mode);
    }

    I32 s__fstat64(I32 fd, I32 statBufPtr) {
        util::getLogger()->debug("S - fstat64 - {} {}", fd, statBufPtr);

        struct stat64 nativeStat{};
        int result = fstat64(fd, &nativeStat);

        if (result < 0) {
            int newErrno = errno;
            return -newErrno;
        }

        writeNativeStatToWasmStat(&nativeStat, statBufPtr);

        return 0;
    }

    I32 s__stat64(I32 pathPtr, I32 statBufPtr) {
        const std::string pathStr = getStringFromWasm(pathPtr);
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->debug("S - stat64 - {} {}", pathStr, statBufPtr);

        // Use virtual FS to do the stat in case file is shared
        struct stat64 nativeStat{};
        storage::SharedFilesManager &sfm = storage::getSharedFilesManager();
        int result = sfm.statFile(pathStr, &nativeStat);

        if (result < 0) {
            return -errno;
        }

        writeNativeStatToWasmStat(&nativeStat, statBufPtr);

        return result;
    }

    I32 s__lstat64(I32 pathPtr, I32 statBufPtr) {
        const std::string fakePath = getMaskedPathFromWasm(pathPtr);
        util::getLogger()->debug("S - lstat - {} {}", fakePath, statBufPtr);

        struct stat64 nativeStat{};
        lstat64(fakePath.c_str(), &nativeStat);
        writeNativeStatToWasmStat(&nativeStat, statBufPtr);

        return 0;
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(wasi, "fd_fdstat_get", I32, wasi_fd_fdstat_get, I32 fd, I32 statPtr) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->debug("S - fd_fdstat_get - {} {}", fd, statPtr);

        storage::FileDescriptor &fileDesc = getExecutingModule()->getFileSystem().getFileDescriptor(fd);
        storage::Stat statResult = fileDesc.stat();

        if (statResult.failed) {
            logger->trace("Failed stat: {}", statResult.wasiErrno);
            return statResult.wasiErrno;
        }

        auto wasiFdStat = &Runtime::memoryRef<__wasi_fdstat_t>(getExecutingModule()->defaultMemory, statPtr);
        wasiFdStat->fs_filetype = statResult.wasiFiletype;
        wasiFdStat->fs_rights_base = fileDesc.getActualRightsBase();
        wasiFdStat->fs_rights_inheriting = fileDesc.getActualRightsInheriting();

        // TODO - set fs flags
        wasiFdStat->fs_flags = 0;

        return 0;
    }

    I32 doFileStat(int fd, const std::string &relativePath, I32 statPtr) {
        WasmModule *module = getExecutingModule();
        storage::FileDescriptor &fileDesc = module->getFileSystem().getFileDescriptor(fd);
        auto wasiFileStat = &Runtime::memoryRef<wasi_filestat_t>(module->defaultMemory, statPtr);

        storage::Stat fileStat = fileDesc.stat(relativePath);
        if (fileStat.failed) {
            return fileStat.wasiErrno;
        }

        wasiFileStat->st_dev = fileStat.st_dev;
        wasiFileStat->st_ino = fileStat.st_ino;
        wasiFileStat->st_filetype = fileStat.wasiFiletype;
        wasiFileStat->st_nlink = fileStat.st_nlink;
        wasiFileStat->st_size = fileStat.st_size;
        wasiFileStat->st_atim = fileStat.st_atim;
        wasiFileStat->st_mtim = fileStat.st_mtim;
        wasiFileStat->st_ctim = fileStat.st_ctim;

        return __WASI_ESUCCESS;
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(wasi, "fd_filestat_get", I32, wasi_fd_filestat_get, I32 fd, I32 statPtr) {
        util::getLogger()->debug("S - fd_filestat_get - {} {}", fd, statPtr);

        return doFileStat(fd, "", statPtr);
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(wasi, "path_filestat_get", I32, wasi_path_filestat_get,
                                   I32 fd, I32 lookupFlags, I32 path, I32 pathLen, I32 statPtr) {
        const std::string &pathStr = getStringFromWasm(path);
        util::getLogger()->debug("S - path_filestat_get - {} {} {} {}", fd, lookupFlags, pathStr, statPtr);

        return doFileStat(fd, pathStr, statPtr);
    }

    /**
     * llseek is called from the implementations of both seek and lseek. Calls
     * to llseek can be replaced with an equivalent call to lseek, which is what
     * musl would do in the absence of llseek.
     */
    I32 s__llseek(I32 fd, I32 offsetHigh, I32 offsetLow, I32 resultPtr, I32 whence) {
        util::getLogger()->debug("S - llseek - {} {} {} {} {}", fd, offsetHigh, offsetLow, resultPtr, whence);

        WasmModule *module = getExecutingModule();
        module->checkThreadOwnsFd(fd);

        // The caller is expecting the result to be written to the result pointer
        Runtime::Memory *memoryPtr = module->defaultMemory;
        I32 *hostResultPtr = &Runtime::memoryRef<I32>(memoryPtr, (Uptr) resultPtr);

        int res = (int) lseek(fd, offsetLow, whence);

        // Success returns zero and failure returns negative errno
        if (res < 0) {
            return -errno;
        } else {
            *hostResultPtr = res;
            return 0;
        }
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(wasi, "fd_tell", I32, wasi_fd_tell, I32 fd, I32 resOffsetPtr) {
        util::getLogger()->debug("S - fd_tell - {} {}", fd, resOffsetPtr);

        WasmModule *module = getExecutingModule();

        storage::FileDescriptor &fileDesc = module->getFileSystem().getFileDescriptor(fd);
        uint64_t offset = fileDesc.tell();
        Runtime::memoryRef<U64>(module->defaultMemory, resOffsetPtr) = offset;
        return 0;
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(wasi, "fd_seek", I32, wasi_fd_seek, I32 fd, I64 offset, I32 whence,
                                   I32 newOffsetPtr) {
        util::getLogger()->debug("S - fd_seek - {} {} {} {}", fd, offset, whence, newOffsetPtr);

        // Get pointer to result in memory
        uint64_t *newOffsetHostPtr = &Runtime::memoryRef<uint64_t>(getExecutingModule()->defaultMemory, newOffsetPtr);

        storage::FileDescriptor &fileDesc = getExecutingModule()->getFileSystem().getFileDescriptor(fd);
        uint16_t wasiErrno = fileDesc.seek(offset, whence, newOffsetHostPtr);

        return wasiErrno;
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "ioctl", I32, ioctl, I32 a, I32 b, I32 c) {
        util::getLogger()->debug("S - ioctl - {} {} {}", a, b, c);

        return 0;
    }

    /**
     * Note here that we assume puts is called on a null-terminated string
     */
    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "puts", I32, puts, I32 strPtr) {
        util::getLogger()->debug("S - puts - {}", strPtr);
        WasmModule *module = getExecutingModule();
        Runtime::Memory *memoryPtr = module->defaultMemory;
        char *hostStr = &Runtime::memoryRef<char>(memoryPtr, (Uptr) strPtr);

        // Capture stdout if necessary
        util::SystemConfig &conf = util::getSystemConfig();
        if (conf.captureStdout == "on") {
            module->captureStdout(hostStr);
        }

        printf("%s\n", hostStr);
        return 0;
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "putc", I32, putc, I32 c, I32 streamPtr) {
        util::getLogger()->debug("S - putc - {} {}", c, streamPtr);

        Runtime::Memory *memoryPtr = getExecutingModule()->defaultMemory;
        FILE *stream = &Runtime::memoryRef<FILE>(memoryPtr, (Uptr) streamPtr);

        putc(c, stream);

        return 0;
    }

    /**
     * fprintf can provide some useful debugging info so we can just spit it to stdout
     */
    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "vfprintf", I32, vfprintf, I32 fd, U32 formatPtr, I32 argList) {
        Runtime::Memory *memoryPtr = getExecutingModule()->defaultMemory;
        const char *format = &Runtime::memoryRef<char>(memoryPtr, (Uptr) formatPtr);
        std::cout << "S - vfprintf - " << format << std::endl;
        return 0;
    }

    I32 s__readlink(I32 pathPtr, I32 bufPtr, I32 bufLen) {
        Runtime::Memory *memoryPtr = getExecutingModule()->defaultMemory;
        char *path = &Runtime::memoryRef<char>(memoryPtr, (Uptr) pathPtr);

        util::getLogger()->debug("S - readlink - {} {} {}", path, bufPtr, bufLen);

        char *buf = Runtime::memoryArrayPtr<char>(memoryPtr, (Uptr) bufPtr, (Uptr) bufLen);

        ssize_t bytesRead = readlink(path, buf, (size_t) bufLen);

        return (I32) bytesRead;
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(wasi, "path_readlink", I32, wasi_path_readlink, I32 rootFd, I32 pathPtr,
                                   I32 pathLen, I32 buffPtr, I32 buffLen, I32 resBytesUsed) {
        std::string pathStr = getStringFromWasm(pathPtr);
        util::getLogger()->debug("S - path_readlink - {} {} {} {} {}", rootFd, pathStr, buffPtr, buffLen, resBytesUsed);

        WasmModule *module = getExecutingModule();
        storage::FileDescriptor &fileDesc = module->getFileSystem().getFileDescriptor(rootFd);

        char *buffer = Runtime::memoryArrayPtr<char>(module->defaultMemory, buffPtr, buffLen);
        ssize_t readBytes = fileDesc.readLink(pathStr, buffer, buffLen);
        Runtime::memoryRef<U32>(module->defaultMemory, resBytesUsed) = (U32) readBytes;

        return __WASI_ESUCCESS;
    }

    // -----------------------------
    // Unsupported
    // -----------------------------

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "umask", I32, umask, I32 a) {
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "fchdir", I32, s__fchdir, I32 a) {
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "chmod", I32, s__chmod, I32 a, I32 b) {
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(wasi, "fd_datasync", I32, wasi_fd_datasync, I32 a) {
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(wasi, "fd_pwrite", I32, wasi_fd_pwrite, I32 a, I32 b, I32 c, I64 d,
                                   I32 e) { throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic); }

    WAVM_DEFINE_INTRINSIC_FUNCTION(wasi, "fd_pread", I32, wasi_fd_pread, I32 a, I32 b, I32 c, I64 d,
                                   I32 e) { throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic); }

    WAVM_DEFINE_INTRINSIC_FUNCTION(wasi, "fd_filestat_set_size", I32, wasi_fd_filestat_set_size, I32 a,
                                   I64 b) { throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic); }

    WAVM_DEFINE_INTRINSIC_FUNCTION(wasi, "fd_sync", I32, wasi_fd_sync, I32 a) {
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(wasi, "fd_allocate", I32, wasi_fd_allocate, I32 a, I64 b, I64 c) {
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(wasi, "fd_advise", I32, wasi_fd_advise, I32 a, I64 b, I64 c, I32 d) {
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(wasi, "fd_fdstat_set_flags", I32, wasi_fd_fdstat_set_flags, I32 a,
                                   I32 b) { throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic); }

    WAVM_DEFINE_INTRINSIC_FUNCTION(wasi, "fd_filestat_set_times", I32, fd_filestat_set_times, I32 a, I64 b, I64 c,
                                   I32 d) { throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic); }

    WAVM_DEFINE_INTRINSIC_FUNCTION(wasi, "path_link", I32, wasi_path_link, I32 a, I32 b, I32 c, I32 d, I32 e, I32 f,
                                   I32 g) { throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic); }

    WAVM_DEFINE_INTRINSIC_FUNCTION(wasi, "path_symlink", I32, wasi_path_symlink, I32 a, I32 b, I32 c, I32 d,
                                   I32 e) { throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic); }

    WAVM_DEFINE_INTRINSIC_FUNCTION(wasi, "path_remove_directory", I32, wasi_path_remove_directory, I32 a, I32 b,
                                   I32 c) { throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic); }


    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "lockf", I32, lockf, I32 a, I32 b, I64 c) {
        util::getLogger()->debug("S - lockf - {} {} {}", a, b, c);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "realpath", I32, realpath, I32 a, U32 b) {
        util::getLogger()->debug("S - realpath - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    // This is an Emscripten-specific function
    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "fiprintf", I32, wasi_fiprintf, I32 a, I32 b, I32 c) {
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    void ioLink() {

    }
}