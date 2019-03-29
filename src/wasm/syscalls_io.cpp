#include "WasmModule.h"
#include "syscalls.h"

#include <util/bytes.h>
#include <util/config.h>

#include <dirent.h>
#include <poll.h>
#include <sys/uio.h>
#include <sys/ioctl.h>

#include <WAVM/Runtime/Runtime.h>
#include <WAVM/Runtime/RuntimeData.h>
#include <WAVM/Runtime/Intrinsics.h>


namespace wasm {

    I32 s__open_alt(I32 argsPtr) {
        Runtime::Memory *memoryPtr = getExecutingModule()->defaultMemory;
        auto args = Runtime::memoryArrayPtr<int>(memoryPtr, argsPtr, 3);
        return s__open(args[0], args[1], args[2]);
    }

    I32 s__open(I32 pathPtr, I32 flags, I32 mode) {
        const std::shared_ptr<spdlog::logger> logger = util::getLogger();
        const std::string path = getStringFromWasm(pathPtr);
        logger->debug("S - open - {} {} {}", path, flags, mode);

        util::SystemConfig &conf = util::getSystemConfig();

        // Check if this is a valid path. Return a read-only handle to the file if so
        int fd;
        std::string fakePath;
        if (path == "/etc/hosts") {
            logger->debug("Opening dummy /etc/hosts");
            fd = open(HOSTS_FILE, 0, 0);

        } else if (path == "/etc/resolv.conf") {
            logger->debug("Opening dummy /etc/resolv.conf");
            fd = open(RESOLV_FILE, 0, 0);

        } else if (path == "/dev/urandom") {
            //TODO avoid use of system-wide urandom
            logger->debug("Opening /dev/urandom");
            fd = open("/dev/urandom", 0, 0);

        } else if (path == "pyvenv.cfg") {
            logger->debug("Forcing non-existent pyvenv.cfg");
            return -ENOENT;
        } else {
            fakePath = maskPath(path.c_str());

            if (conf.unsafeMode == "on") {
                logger->debug("Opening {}", fakePath);
                fd = open(fakePath.c_str(), flags, mode);
            } else {
                logger->error("Opening arbitrary path {}", fakePath);
                throw std::runtime_error("Opening arbitrary path");
            }
        }

        if (fd > 0) {
            addFdForThisThread(fd);
        } else {
            // This is an error of some form
            if (!fakePath.empty()) {
                logger->error("Failed on open - {} (masked {})", path, fakePath);
            } else {
                logger->error("Failed on open - {}", path);
            }
        }

        return fd;
    }

    I32 s__dup(I32 oldFd) {
        util::getLogger()->debug("S - dup - {}", oldFd);

        checkThreadOwnsFd(oldFd);

        int newFd = dup(oldFd);
        addFdForThisThread(newFd);

        return newFd;
    }

    I32 s__fcntl64(I32 fd, I32 cmd, I32 c) {
        util::getLogger()->debug("S - fcntl64 - {} {} {}", fd, cmd, c);

        checkThreadOwnsFd(fd);

        // Allow for now
        // TODO - is this ok?
        fcntl(fd, cmd, c);
        return 0;
    }

    /**
     * This function is tricky to implement as it's iterating through an unknown number of
     * files in the directory. We will be running two loops, one to fill up the given buffer,
     * and a nested loop to iterate through the native directory listing.
     *
     * We try to be conservative but will throw an exception if things aren't right. A bug here
     * can be hard to find.
     */
    I32 s__getdents64(I32 fd, I32 wasmDirentBuf, I32 wasmDirentBufLen) {
        util::getLogger()->debug("S - getdents64 - {} {} {}", fd, wasmDirentBuf, wasmDirentBufLen);

        Runtime::Memory *memoryPtr = getExecutingModule()->defaultMemory;
        U8 *hostWasmDirentBuf = &Runtime::memoryRef<U8>(memoryPtr, (Uptr) wasmDirentBuf);

        // Work out how many dirents we can fit (rounding down)
        size_t wasmDirentSize = sizeof(wasm_dirent64);

        // Create a small native buffer (can't overshoot the wasm offset)
        int nativeBufLen = 60;

        U32 wasmBytesRead;
        int wasmDirentCount = 0;

        // Here we will iterate getting native dirents until we've filled up the wasm buffer supplied
        for (wasmBytesRead = 0; wasmBytesRead < wasmDirentBufLen - (2 * wasmDirentSize);) {

            // Make the native syscall. This will read in a list of dirent structs to the buffer
            // We need to read at most two native dirents. Each one will be at most 40 bytes.
            auto nativeBuf = new char[nativeBufLen];
            long nativeBytesRead = syscall(SYS_getdents64, fd, nativeBuf, nativeBufLen);

            if (nativeBytesRead == 0) {
                // No more native dirents
                return wasmBytesRead;
            } else if (nativeBytesRead < 0) {
                // Error reading native dirents
                return (I32) nativeBytesRead;
            }

            // Now we iterate through the dirents we just got back from the host
            int nativeOffset;
            for (nativeOffset = 0; nativeOffset < nativeBytesRead;) {
                // If we're going to overshoot on the wasm buffer, we have a problem
                if (wasmBytesRead > wasmDirentBufLen) {
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
                auto dWasmBytes = reinterpret_cast<uint8_t *>(&dWasm);
                std::copy(dWasmBytes, dWasmBytes + wasmDirentSize, hostWasmDirentBuf + wasmBytesRead);

                // Move offsets along
                nativeOffset += d->d_reclen;

                wasmBytesRead += wasmDirentSize;
                wasmDirentCount++;
            }
        }

        return wasmBytesRead;
    }

    I32 s__getcwd(I32 bufPtr, I32 bufLen) {
        util::getLogger()->debug("S - getcwd - {} {}", bufPtr, bufLen);

        // Do nothing here, we want an empty working dir.

        return 0;
    }

    /**
     * Read is ok provided the function owns the file descriptor
     */
    I32 s__read(I32 fd, I32 bufPtr, I32 bufLen){
        util::getLogger()->debug("S - read - {} {} {}", fd, bufPtr, bufLen);

        // Provided the thread owns the fd, we allow reading.
        checkThreadOwnsFd(fd);

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
        checkThreadOwnsFd(fd);
        removeFdForThisThread(fd);

        close(fd);

        return 0;
    }

    I32 s__poll_alt(I32 argsPtr) {
        Runtime::GCPointer<Runtime::Memory> &memoryPtr = getExecutingModule()->defaultMemory;
        auto args = Runtime::memoryArrayPtr<int>(memoryPtr, (Uptr) argsPtr, (Uptr) 3);

        return s__poll(args[0], args[1], args[2]);
    }
    
    /** Poll is ok but can pass in an array of structs. */
    I32 s__poll(I32 fdsPtr, I32 nfds, I32 timeout) {
        util::getLogger()->debug("S - poll - {} {} {}", fdsPtr, nfds, timeout);

        auto *fds = Runtime::memoryArrayPtr<pollfd>(getExecutingModule()->defaultMemory, (Uptr) fdsPtr, (Uptr) nfds);

        // Check this thread has permission to poll the relevant fds
        for (int i = 0; i < nfds; i++) {
            pollfd fd = fds[i];
            checkThreadOwnsFd(fd.fd);
        }

        int pollRes = poll(fds, (size_t) nfds, timeout);
        return pollRes;
    }

    I32 s__ioctl(I32 fd, I32 request, I32 argPtr, I32 d, I32 e, I32 f) {
        util::getLogger()->debug("S - ioctl - {} {} {} {} {} {}", fd, request, argPtr, d, e, f);

        auto hostArgPtr = &Runtime::memoryRef<U8>(getExecutingModule()->defaultMemory, (Uptr) argPtr);

        checkThreadOwnsFd(fd);

        int res = ioctl(fd, request, hostArgPtr);

        return res;
    }

    /**
     * Writing is ok provided the function owns the file descriptor
     */
    I32 s__writev(I32 fd, I32 iov, I32 iovcnt) {
        util::getLogger()->debug("S - writev - {} {} {}", fd, iov, iovcnt);

        checkThreadOwnsFd(fd);

        iovec *nativeIovecs = wasmIovecsToNativeIovecs(iov, iovcnt);

        Iptr count = writev(STDOUT_FILENO, nativeIovecs, iovcnt);
        fflush(stdout);

        delete[] nativeIovecs;

        return (I32) count;
    }

    I32 s__readv(I32 fd, I32 iovecPtr, I32 iovecCount) {
        util::getLogger()->debug("S - readv - {} {} {}", fd, iovecPtr, iovecCount);

        checkThreadOwnsFd(fd);

        iovec *nativeIovecs = wasmIovecsToNativeIovecs(iovecPtr, iovecCount);

        int bytesRead = readv(fd, nativeIovecs, iovecCount);

        delete[] nativeIovecs;

        return bytesRead;
    }

    /**
     * Writing is ok provided the function owns the file descriptor
     */
    I32 s__write(I32 fd, I32 bufPtr, I32 bufLen) {
        util::getLogger()->debug("S - write - {} {} {}", fd, bufPtr, bufLen);

        // Provided the thread owns the fd, we allow reading.
        checkThreadOwnsFd(fd);

        // Get the buffer etc.
        Runtime::Memory *memoryPtr = getExecutingModule()->defaultMemory;
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

    /**
     *  We don't want to give away any real info about the filesystem, but we can just return the default
     *  stat object and catch the application later if it tries to do anything dodgy.
     */
    I32 s__fstat64(I32 fd, I32 statBufPtr) {
        util::getLogger()->debug("S - fstat64 - {} {}", fd, statBufPtr);

        struct stat64 nativeStat{};
        fstat64(fd, &nativeStat);
        writeNativeStatToWasmStat(&nativeStat, statBufPtr);

        return 0;
    }

    I32 s__stat64(I32 pathPtr, I32 statBufPtr) {
        // Get the path
        const std::string fakePath = getMaskedPathFromWasm(pathPtr);

        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->debug("S - stat64 - {} {}", fakePath, statBufPtr);

        struct stat64 nativeStat{};
        int result = stat64(fakePath.c_str(), &nativeStat);
        if(result != 0) {
            logger->debug("stat64 errno: {}", errno);
        }

        if(result == 0) {
            writeNativeStatToWasmStat(&nativeStat, statBufPtr);
        }

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
     * Although llseek is being called, musl is using it within the implementation of lseek,
     * therefore we can just use lseek as a shortcut
     */
    I32 s__llseek(I32 fd, I32 offsetHigh, I32 offsetLow, I32 resultPtr, I32 whence) {
        util::getLogger()->debug("S - llseek - {} {} {} {} {}", fd, offsetHigh, offsetLow, resultPtr, whence);

        checkThreadOwnsFd(fd);

        int res = (int) lseek(fd, offsetLow, whence);

        return (I32) res;
    }

    DEFINE_INTRINSIC_FUNCTION(env, "ioctl", I32, ioctl, I32 a, I32 b, I32 c) {
        util::getLogger()->debug("S - ioctl - {} {} {}", a, b, c);


        return 0;
    }

    DEFINE_INTRINSIC_FUNCTION(env, "puts", I32, puts, I32 strPtr) {
        Runtime::Memory *memoryPtr = getExecutingModule()->defaultMemory;
        char *string = &Runtime::memoryRef<char>(memoryPtr, (Uptr) strPtr);

        util::getLogger()->debug("S - puts {}", string);

        return 0;
    }

    /**
     * fprintf can provide some useful debugging info so we can just spit it to stdout
     */
    DEFINE_INTRINSIC_FUNCTION(env, "vfprintf", I32, vfprintf, I32 fd, U32 formatPtr, I32 argList) {
        Runtime::Memory *memoryPtr = getExecutingModule()->defaultMemory;
        const char *format = &Runtime::memoryRef<char>(memoryPtr, (Uptr) formatPtr);
        std::cout << "S - vfprintf - " << format << std::endl;
        return 0;
    }
}