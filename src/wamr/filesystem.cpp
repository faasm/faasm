#include <conf/FaasmConfig.h>
#include <faabric/util/logging.h>
#include <storage/FileDescriptor.h>
#include <wamr/WAMRWasmModule.h>
#include <wamr/native.h>
#include <wamr/types.h>

#include <cstring>
#include <stdexcept>
#include <string>
#include <sys/uio.h>

#include <wasm_export.h>
#include <wasmtime_ssp.h>

namespace wasm {
uint32_t doWasiDup(uint32_t fd)
{
    storage::FileSystem& fs = getExecutingWAMRModule()->getFileSystem();
    return fs.dup(fd);
}

static uint32_t __wasi_fd_dup_wrapper(wasm_exec_env_t exec_env,
                                      uint32_t fd,
                                      uint32_t* resFd)
{
    SPDLOG_DEBUG("S - __wasi_fd_dup {}");

    *resFd = doWasiDup(fd);

    return __WASI_ESUCCESS;
}

static uint32_t dup_wrapper(wasm_exec_env_t exec_env, uint32_t fd)
{
    SPDLOG_DEBUG("S - dup {}", fd);
    return doWasiDup(fd);
}

static uint32_t getpwnam_wrapper(wasm_exec_env_t exec_env, uint32_t a)
{
    WAMR_NATIVE_SYMBOL_NOT_IMPLEMENTED("getpwnam");

    return 0;
}

static int32_t sendfile_wrapper(wasm_exec_env_t exec_env,
                                int32_t out_fd,
                                int32_t in_fd,
                                int32_t offset,
                                int32_t count)
{
    WAMR_NATIVE_SYMBOL_NOT_IMPLEMENTED("sendfile");

    return 0;
}

static int32_t tempnam_wrapper(wasm_exec_env_t exec_env, int32_t a, int32_t b)
{
    WAMR_NATIVE_SYMBOL_NOT_IMPLEMENTED("tempnam");

    return 0;
}

static NativeSymbol ns[] = {
    REG_NATIVE_FUNC(__wasi_fd_dup, "(i*)i"),
    REG_NATIVE_FUNC(dup, "(i)i"),
    REG_NATIVE_FUNC(getpwnam, "(i)i"),
    REG_NATIVE_FUNC(sendfile, "(iiii)i"),
    REG_NATIVE_FUNC(tempnam, "(ii)i"),
};

uint32_t getFaasmFilesystemApi(NativeSymbol** nativeSymbols)
{
    *nativeSymbols = ns;
    return sizeof(ns) / sizeof(NativeSymbol);
}

// ---------- WASI symbols ----------

static uint32_t wasi_fd_allocate(wasm_exec_env_t exec_env,
                                 __wasi_fd_t fd,
                                 __wasi_filesize_t offset,
                                 __wasi_filesize_t len)
{
    WAMR_NATIVE_SYMBOL_NOT_IMPLEMENTED("wasi_fd_allocate");

    return 0;
}

static int32_t wasi_fd_close(wasm_exec_env_t exec_env, int32_t fd)
{
    SPDLOG_DEBUG("S - fd_close {}", fd);

    // Ignore for now

    return 0;
}

static int32_t wasi_fd_fdstat_get(wasm_exec_env_t exec_env,
                                  int32_t fd,
                                  __wasi_fdstat_t* statWasm)
{
    WAMRWasmModule* module = getExecutingWAMRModule();
    storage::FileSystem& fs = module->getFileSystem();
    std::string path = fs.getPathForFd(fd);
    SPDLOG_DEBUG("S - fd_fdstat_get {} ({})", fd, path);

    if (!fs.fileDescriptorExists(fd)) {
        return __WASI_EBADF;
    }

    module->validateNativePointer(statWasm, sizeof(__wasi_fdstat_t));

    storage::FileDescriptor& fileDesc = fs.getFileDescriptor(fd);
    storage::Stat statNative = fileDesc.stat();

    if (statNative.failed) {
        SPDLOG_ERROR("Failed stat: {}", statNative.wasiErrno);
        return statNative.wasiErrno;
    }

    statWasm->fs_filetype = statNative.wasiFiletype;
    statWasm->fs_rights_base = fileDesc.getActualRightsBase();
    statWasm->fs_rights_inheriting = fileDesc.getActualRightsInheriting();

    // TODO - set fs flags
    statWasm->fs_flags = 0;

    return __WASI_ESUCCESS;
}

static int32_t wasi_fd_fdstat_set_flags(wasm_exec_env_t exec_env,
                                        int32_t a,
                                        int32_t b)
{
    WAMR_NATIVE_SYMBOL_NOT_IMPLEMENTED("wasi_fd_fdstat_set_flags");

    return 0;
}

static int32_t wasi_fd_fdstat_set_rights(wasm_exec_env_t exec_env,
                                         int32_t a,
                                         int64_t b,
                                         int64_t c)
{
    WAMR_NATIVE_SYMBOL_NOT_IMPLEMENTED("wasi_fd_fdstat_set_rights");

    return 0;
}

static int32_t doFileStat(uint32_t fd,
                          const std::string& relativePath,
                          __wasi_filestat_t* statWasm)
{
    WAMRWasmModule* module = getExecutingWAMRModule();
    storage::FileDescriptor& fileDesc =
      module->getFileSystem().getFileDescriptor(fd);

    storage::Stat statNative = fileDesc.stat(relativePath);
    if (statNative.failed) {
        SPDLOG_ERROR("Failed stat: {}", statNative.wasiErrno);
        return statNative.wasiErrno;
    }

    module->validateNativePointer(statWasm, sizeof(__wasi_filestat_t));
    statWasm->st_dev = statNative.st_dev;
    statWasm->st_ino = statNative.st_ino;
    statWasm->st_filetype = statNative.wasiFiletype;
    statWasm->st_nlink = statNative.st_nlink;
    statWasm->st_size = statNative.st_size;
    statWasm->st_atim = statNative.st_atim;
    statWasm->st_mtim = statNative.st_mtim;
    statWasm->st_ctim = statNative.st_ctim;

    return __WASI_ESUCCESS;
}

static int32_t wasi_fd_filestat_get(wasm_exec_env_t exec_env,
                                    int32_t fd,
                                    __wasi_filestat_t* statWasm)
{
    SPDLOG_DEBUG("S - fd_filestat_get {}", fd);

    return doFileStat(fd, "", statWasm);
}

static int32_t wasi_fd_filestat_set_size(wasm_exec_env_t execEnv,
                                         int32_t a,
                                         int64_t b)
{
    WAMR_NATIVE_SYMBOL_NOT_IMPLEMENTED("wasi_fd_filestat_set_size");

    return 0;
}

static uint32_t wasi_fd_pread(wasm_exec_env_t exec_env,
                              __wasi_fd_t fd,
                              iovec_app_t* iovecWasm,
                              uint32_t iovecLen,
                              __wasi_filesize_t offset,
                              uint32_t* nReadWasm)
{
    WAMR_NATIVE_SYMBOL_NOT_IMPLEMENTED("wasi_fd_pread");

    return 0;
}

static int32_t wasi_fd_prestat_dir_name(wasm_exec_env_t exec_env,
                                        int32_t fd,
                                        char* path,
                                        int32_t* pathLen)
{
    SPDLOG_DEBUG("S - fd_prestat_dir_name {}", fd);

    WAMRWasmModule* module = getExecutingWAMRModule();
    if (!module->getFileSystem().fileDescriptorExists(fd)) {
        return __WASI_EBADF;
    }

    storage::FileDescriptor& fileDesc =
      module->getFileSystem().getFileDescriptor(fd);
    module->writeStringToWasmMemory(fileDesc.getPath(), path);

    return __WASI_ESUCCESS;
}

static int32_t wasi_fd_prestat_get(wasm_exec_env_t exec_env,
                                   int32_t fd,
                                   wasi_prestat_app_t* prestatWasm)
{
    SPDLOG_DEBUG("S - fd_prestat_get {}", fd);

    WAMRWasmModule* module = getExecutingWAMRModule();
    if (!module->getFileSystem().fileDescriptorExists(fd)) {
        return __WASI_EBADF;
    }

    module->validateNativePointer(prestatWasm, sizeof(wasi_prestat_app_t));

    storage::FileDescriptor& fileDesc =
      module->getFileSystem().getFileDescriptor(fd);

    prestatWasm->pr_type = fileDesc.wasiPreopenType;
    prestatWasm->pr_name_len = fileDesc.getPath().size();

    return __WASI_ESUCCESS;
}

static uint32_t wasi_fd_pwrite(wasm_exec_env_t exec_env,
                               __wasi_fd_t fd,
                               const iovec_app_t* iovecWasm,
                               uint32_t iovecLen,
                               __wasi_filesize_t offset,
                               uint32_t* nWrittenWasm)
{
    WAMR_NATIVE_SYMBOL_NOT_IMPLEMENTED("wasi_fd_pwrite");

    return 0;
}

static int32_t wasi_fd_read(wasm_exec_env_t exec_env,
                            int32_t fd,
                            iovec_app_t* ioVecBuffWasm,
                            int32_t ioVecCountWasm,
                            int32_t* bytesRead)
{
    WAMRWasmModule* module = getExecutingWAMRModule();
    storage::FileSystem& fileSystem = module->getFileSystem();
    std::string path = fileSystem.getPathForFd(fd);

    SPDLOG_TRACE("S - fd_read {} ({})", fd, path);

    storage::FileDescriptor fileDesc = fileSystem.getFileDescriptor(fd);

    // Translate app iovecs to native ones
    std::vector<::iovec> ioVecBuffNative(ioVecCountWasm, (::iovec){});
    for (int i = 0; i < ioVecCountWasm; i++) {
        module->validateWasmOffset(ioVecBuffWasm[i].buffOffset,
                                   sizeof(char) * ioVecBuffWasm[i].buffLen);

        ioVecBuffNative[i] = {
            .iov_base =
              module->wasmPointerToNative(ioVecBuffWasm[i].buffOffset),
            .iov_len = ioVecBuffWasm[i].buffLen,
        };
    }

    // Read from fd
    module->validateNativePointer(bytesRead, sizeof(int32_t));
    *bytesRead =
      ::readv(fileDesc.getLinuxFd(), ioVecBuffNative.data(), ioVecCountWasm);

    return __WASI_ESUCCESS;
}

static int32_t wasi_fd_readdir(wasm_exec_env_t exec_env,
                               int32_t a,
                               int32_t* b,
                               char* c,
                               int64_t d,
                               int32_t e)
{
    WAMR_NATIVE_SYMBOL_NOT_IMPLEMENTED("fd_readdir");

    return 0;
}

static int32_t wasi_fd_seek(wasm_exec_env_t exec_env,
                            int32_t fd,
                            int64_t offset,
                            int32_t whence,
                            __wasi_filesize_t* newOffset)
{
    SPDLOG_DEBUG("S - fd_seek {} {} {}", fd, offset, whence);

    WAMRWasmModule* module = getExecutingWAMRModule();
    module->validateNativePointer(newOffset, sizeof(__wasi_filesize_t));

    storage::FileDescriptor& fileDesc =
      module->getFileSystem().getFileDescriptor(fd);
    auto wasiErrno = fileDesc.seek(offset, whence, newOffset);

    return wasiErrno;
}

static uint32_t wasi_fd_sync(wasm_exec_env_t exec_env, __wasi_fd_t fd)
{
    WAMR_NATIVE_SYMBOL_NOT_IMPLEMENTED("wasi_fd_sync");

    return 0;
}

static uint32_t wasi_fd_tell(wasm_exec_env_t exec_env,
                             uint32_t fd,
                             uint32_t* resOffset)
{
    SPDLOG_TRACE("S - fd_tell {}", fd);

    storage::FileDescriptor& fileDesc =
      getExecutingWAMRModule()->getFileSystem().getFileDescriptor(fd);
    *resOffset = fileDesc.tell();

    return 0;
}

static int32_t wasi_fd_write(wasm_exec_env_t exec_env,
                             int32_t fd,
                             iovec_app_t* ioVecBuffWasm,
                             int32_t ioVecCountWasm,
                             int32_t* bytesWritten)
{
    WAMRWasmModule* module = getExecutingWAMRModule();
    storage::FileSystem& fileSystem = module->getFileSystem();
    std::string path = fileSystem.getPathForFd(fd);

    SPDLOG_TRACE("S - fd_write {} ({})", fd, path);

    // Check pointers
    module->validateNativePointer(reinterpret_cast<void*>(ioVecBuffWasm),
                                  sizeof(iovec_app_t) * ioVecCountWasm);
    module->validateNativePointer(bytesWritten, sizeof(int32_t));

    // Translate the app iovecs into native iovecs
    std::vector<::iovec> ioVecBuffNative(ioVecCountWasm, (::iovec){});
    for (int i = 0; i < ioVecCountWasm; i++) {
        module->validateWasmOffset(ioVecBuffWasm[i].buffOffset,
                                   sizeof(char) * ioVecBuffWasm[i].buffLen);

        ioVecBuffNative[i] = {
            .iov_base =
              module->wasmPointerToNative(ioVecBuffWasm[i].buffOffset),
            .iov_len = ioVecBuffWasm[i].buffLen,
        };
    }

    // Do the write
    storage::FileDescriptor& fileDesc = fileSystem.getFileDescriptor(fd);
    ssize_t n = fileDesc.write(ioVecBuffNative, ioVecCountWasm);
    if (n < 0) {
        SPDLOG_ERROR(
          "writev failed on fd {}: {}", fileDesc.getLinuxFd(), strerror(errno));
    }

    // Write number of bytes to wasm
    *bytesWritten = n;

    // Capture stdout if needed
    conf::FaasmConfig& conf = conf::getFaasmConfig();
    bool isStd = fd <= 2;
    if (isStd && conf.captureStdout == "on") {
        module->captureStdout(ioVecBuffNative.data(), ioVecCountWasm);
    }

    return __WASI_ESUCCESS;
}

static int32_t wasi_path_create_directory(wasm_exec_env_t exec_env,
                                          int32_t a,
                                          int32_t* b,
                                          char* c)
{
    WAMR_NATIVE_SYMBOL_NOT_IMPLEMENTED("wasi_path_create_directory");

    return 0;
}

static int32_t wasi_path_filestat_get(wasm_exec_env_t exec_env,
                                      int32_t fd,
                                      int32_t lookupFlags,
                                      char* path,
                                      int32_t pathLen,
                                      __wasi_filestat_t* statWasm)
{
    WAMRWasmModule* module = getExecutingWAMRModule();
    module->validateNativePointer(path, pathLen);
    std::string pathStr(path, pathLen);

    SPDLOG_DEBUG("S - path_filestat_get {} ({})", fd, pathStr, path, pathLen);

    return doFileStat(fd, pathStr, statWasm);
}

static uint32_t wasi_path_filestat_set_times(wasm_exec_env_t exec_env,
                                             __wasi_fd_t fd,
                                             __wasi_lookupflags_t flags,
                                             const char* path,
                                             uint32_t pathLen,
                                             __wasi_timestamp_t stAtim,
                                             __wasi_timestamp_t stMtim,
                                             __wasi_fstflags_t fstflags)
{
    WAMR_NATIVE_SYMBOL_NOT_IMPLEMENTED("wasi_path_filesat_set_times");

    return 0;
}

static int32_t wasi_path_link(wasm_exec_env_t exec_env,
                              int32_t a,
                              int32_t b,
                              int32_t* c,
                              char* d,
                              int32_t e,
                              int32_t* f,
                              char* g)
{
    WAMR_NATIVE_SYMBOL_NOT_IMPLEMENTED("wasi_path_link");

    return 0;
}

static int32_t wasi_path_open(wasm_exec_env_t exec_env,
                              int32_t fdNative,
                              int32_t lookupFlags,
                              char* path,
                              int32_t pathLen,
                              int32_t openFlags,
                              int64_t rightsBase,
                              int64_t rightsInheriting,
                              int32_t fdFlags,
                              int32_t* fdWasm)
{
    WAMRWasmModule* module = getExecutingWAMRModule();

    module->validateNativePointer(path, pathLen);
    const std::string pathStr(path, pathLen);

    SPDLOG_DEBUG("S - path_open {} {} {}", fdNative, pathStr, pathLen);

    module->validateNativePointer(fdWasm, sizeof(int32_t));
    *fdWasm = module->getFileSystem().openFileDescriptor(fdNative,
                                                         pathStr,
                                                         rightsBase,
                                                         rightsInheriting,
                                                         lookupFlags,
                                                         openFlags,
                                                         fdFlags);

    if (*fdWasm < 0) {
        return -1 * *fdWasm;
    }

    return __WASI_ESUCCESS;
}

static int32_t wasi_path_readlink(wasm_exec_env_t exec_env,
                                  uint32_t fd,
                                  char* path,
                                  uint32_t pathLen,
                                  char* buf,
                                  uint32_t bufLen,
                                  uint32_t* resBytesUsed)
{
    WAMRWasmModule* module = getExecutingWAMRModule();

    module->validateNativePointer(path, pathLen);
    const std::string pathStr(path, pathLen);

    SPDLOG_DEBUG("S - path_readlink {} {} {}", fd, pathStr, pathLen);

    storage::FileDescriptor& fileDesc =
      module->getFileSystem().getFileDescriptor(fd);

    *resBytesUsed = fileDesc.readLink(pathStr, buf, bufLen);

    return __WASI_ESUCCESS;
}

static int32_t wasi_path_remove_directory(wasm_exec_env_t exec_env,
                                          int32_t a,
                                          int32_t* b,
                                          char* c)
{
    WAMR_NATIVE_SYMBOL_NOT_IMPLEMENTED("wasi_path_remove_directory");

    return 0;
}

static int32_t wasi_path_rename(wasm_exec_env_t exec_env,
                                uint32_t oldFd,
                                char* oldPath,
                                uint32_t oldPathLen,
                                uint32_t newFd,
                                char* newPath,
                                uint32_t newPathLen)
{
    SPDLOG_DEBUG("S - path_rename {} (fd: {}) -> {} (fd: {})",
                 oldPath,
                 oldFd,
                 newPath,
                 newFd);

    std::string oldPathStr(oldPath, oldPathLen);
    std::string newPathStr(newPath, newPathLen);

    storage::FileSystem& fs = getExecutingWAMRModule()->getFileSystem();
    storage::FileDescriptor& oldFileDesc = fs.getFileDescriptor(oldFd);
    storage::FileDescriptor& newFileDesc = fs.getFileDescriptor(newFd);

    const std::string& fullNewPath = newFileDesc.absPath(newPathStr);
    bool success = oldFileDesc.rename(fullNewPath, oldPathStr);
    if (!success) {
        return oldFileDesc.getWasiErrno();
    }

    return __WASI_ESUCCESS;
}

static int32_t wasi_path_symlink(wasm_exec_env_t exec_env,
                                 const char* oldPath,
                                 uint32_t oldPathLen,
                                 uint32_t fd,
                                 const char* newPath,
                                 uint32_t newPathLen)
{
    WAMR_NATIVE_SYMBOL_NOT_IMPLEMENTED("wasi_path_symlink");

    return 0;
}

static int32_t wasi_path_unlink_file(wasm_exec_env_t exec_env,
                                     uint32_t fd,
                                     char* path,
                                     uint32_t pathLen)
{
    SPDLOG_DEBUG("S - path_unlink_file");

    std::string pathStr(path, pathLen);
    storage::FileDescriptor& fileDesc =
      getExecutingWAMRModule()->getFileSystem().getFileDescriptor(fd);
    bool success = fileDesc.unlink(pathStr);

    if (!success) {
        return fileDesc.getWasiErrno();
    }

    return __WASI_ESUCCESS;
}

static NativeSymbol wasiNs[] = {
    REG_WASI_NATIVE_FUNC(fd_allocate, "(iII)i"),
    REG_WASI_NATIVE_FUNC(fd_close, "(i)i"),
    REG_WASI_NATIVE_FUNC(fd_fdstat_get, "(i*)i"),
    REG_WASI_NATIVE_FUNC(fd_fdstat_set_flags, "(ii)i"),
    REG_WASI_NATIVE_FUNC(fd_fdstat_set_rights, "(iII)i"),
    REG_WASI_NATIVE_FUNC(fd_filestat_get, "(i*)i"),
    REG_WASI_NATIVE_FUNC(fd_filestat_set_size, "(iI)i"),
    REG_WASI_NATIVE_FUNC(fd_pread, "(i*iI*)i"),
    REG_WASI_NATIVE_FUNC(fd_prestat_dir_name, "(i*~)i"),
    REG_WASI_NATIVE_FUNC(fd_prestat_get, "(i*)i"),
    REG_WASI_NATIVE_FUNC(fd_read, "(i*i*)i"),
    REG_WASI_NATIVE_FUNC(fd_readdir, "(i*~I*)i"),
    REG_WASI_NATIVE_FUNC(fd_pwrite, "(i*iI*)i"),
    REG_WASI_NATIVE_FUNC(fd_seek, "(iIi*)i"),
    REG_WASI_NATIVE_FUNC(fd_sync, "(i)i"),
    REG_WASI_NATIVE_FUNC(fd_tell, "(i*)i"),
    REG_WASI_NATIVE_FUNC(fd_write, "(i*i*)i"),
    REG_WASI_NATIVE_FUNC(path_create_directory, "(i*~)i"),
    REG_WASI_NATIVE_FUNC(path_filestat_get, "(ii*~*)i"),
    REG_WASI_NATIVE_FUNC(path_filestat_set_times, "(ii*~IIi)i"),
    REG_WASI_NATIVE_FUNC(path_link, "(ii*~i*~)i"),
    REG_WASI_NATIVE_FUNC(path_open, "(ii*~iIIi*)i"),
    REG_WASI_NATIVE_FUNC(path_readlink, "(i*~*~*)i"),
    REG_WASI_NATIVE_FUNC(path_remove_directory, "(i*~)i"),
    REG_WASI_NATIVE_FUNC(path_rename, "(i*~i*~)i"),
    REG_WASI_NATIVE_FUNC(path_symlink, "(*~i*~)i"),
    REG_WASI_NATIVE_FUNC(path_unlink_file, "(i*~)i"),
};

uint32_t getFaasmWasiFilesystemApi(NativeSymbol** nativeSymbols)
{
    *nativeSymbols = wasiNs;
    return sizeof(wasiNs) / sizeof(NativeSymbol);
}
}
