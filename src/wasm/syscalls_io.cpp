#include "WasmModule.h"
#include "syscalls.h"

#include <util/bytes.h>
#include <util/config.h>

#include <dirent.h>
#include <poll.h>
#include <sys/uio.h>
#include <sys/ioctl.h>
#include <sys/unistd.h>
#include <sys/mman.h>

#include <WAVM/Runtime/Runtime.h>
#include <WAVM/Runtime/Intrinsics.h>
#include <storage/FileLoader.h>
#include <storage/SharedFilesManager.h>
#include <util/macros.h>


namespace wasm {

    I32 s__open(I32 pathPtr, I32 flags, I32 mode) {
        const std::shared_ptr<spdlog::logger> logger = util::getLogger();
        const std::string path = getStringFromWasm(pathPtr);
        logger->debug("S - open - {} {} {}", path, flags, mode);

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

    I32 s__dup(I32 oldFd) {
        util::getLogger()->debug("S - dup - {}", oldFd);

        WasmModule *module = getExecutingModule();

        module->checkThreadOwnsFd(oldFd);

        int newFd = dup(oldFd);
        module->addFdForThisThread(newFd);

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
                    dWasm.d_off = wasmDirentCount * wasmDirentSize;
                    dWasm.d_type = d->d_type;
                    dWasm.d_reclen = wasmDirentSize;

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

    I32 s__getcwd(I32 bufPtr, I32 bufLen) {
        util::getLogger()->debug("S - getcwd - {} {}", bufPtr, bufLen);

        Runtime::Memory *memoryPtr = getExecutingModule()->defaultMemory;

        char *hostBuf = Runtime::memoryArrayPtr<char>(memoryPtr, (Uptr) bufPtr, (Uptr) bufLen);

        // Fake working directory
        std::strcpy(hostBuf, FAKE_WORKING_DIR);

        // Note, this needs to return the buffer on success, NOT zero
        return bufPtr;
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

    /**
    * readlink is ok for certain special cases
    */
    I32 s__readlink(I32 pathPtr, I32 bufPtr, I32 bufLen) {
        Runtime::Memory *memoryPtr = getExecutingModule()->defaultMemory;
        char *path = &Runtime::memoryRef<char>(memoryPtr, (Uptr) pathPtr);

        util::getLogger()->debug("S - readlink - {} {} {}", path, bufPtr, bufLen);

        char *buf = Runtime::memoryArrayPtr<char>(memoryPtr, (Uptr) bufPtr, (Uptr) bufLen);

        ssize_t bytesRead = readlink(path, buf, (size_t) bufLen);

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

    /**
     * Writing is ok provided the function owns the file descriptor
     */
    I32 s__writev(I32 fd, I32 iov, I32 iovcnt) {
        util::getLogger()->debug("S - writev - {} {} {}", fd, iov, iovcnt);

        WasmModule *module = getExecutingModule();
        module->checkThreadOwnsFd(fd);

        iovec *nativeIovecs = wasmIovecsToNativeIovecs(iov, iovcnt);
        Iptr count = writev(fd, nativeIovecs, iovcnt);

        // Catpure stdout if necessary, otherwise write as normal
        util::SystemConfig &conf = util::getSystemConfig();
        if (fd == STDOUT_FILENO && conf.captureStdout == "on") {
            module->captureStdout(nativeIovecs, iovcnt);
        }


        delete[] nativeIovecs;

        return (I32) count;
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
}