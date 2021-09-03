#include <conf/FaasmConfig.h>
#include <faabric/util/logging.h>
#include <storage/FileDescriptor.h>
#include <wamr/WAMRWasmModule.h>
#include <wamr/native.h>

#include <cstring>
#include <stdexcept>
#include <sys/uio.h>

// WAMR includes
#include <wasm_export.h>
#include <wasmtime_ssp.h>

namespace wasm {
static int32_t __wasi_fd_dup_wrapper(wasm_exec_env_t exec_env, int32_t a)
{
    throw std::runtime_error("Native WASI fd dup not implemented");
}

static int32_t tempnam_wrapper(wasm_exec_env_t exec_env, int32_t a, int32_t b)
{
    throw std::runtime_error("tempnam not implemented");
}

static NativeSymbol ns[] = {
    REG_NATIVE_FUNC(__wasi_fd_dup, "(i*)i"),
    REG_NATIVE_FUNC(tempnam, "(ii)i"),
};

uint32_t getFaasmFilesystemApi(NativeSymbol** nativeSymbols)
{
    *nativeSymbols = ns;
    return sizeof(ns) / sizeof(NativeSymbol);
}

// ---------- WASI symbols ----------

static int32_t wasi_fd_close(wasm_exec_env_t exec_env, int32_t fd)
{
    SPDLOG_DEBUG("S - fd_close {}", fd);

    // Ignore for now

    return 0;
}

static int32_t wasi_fd_fdstat_get(wasm_exec_env_t exec_env,
                                  int32_t fd,
                                  __wasi_fdstat_t* statApp)
{
    std::string path =
      getExecutingWAMRModule()->getFileSystem().getPathForFd(fd);
    SPDLOG_DEBUG("S - fd_fdstat_get {} ({})", fd, path);

    WAMRWasmModule* module = getExecutingWAMRModule();
    if (!module->getFileSystem().fileDescriptorExists(fd)) {
        return __WASI_EBADF;
    }

    VALIDATE_NATIVE_ADDR(statApp, sizeof(__wasi_fdstat_t))

    storage::FileDescriptor& fileDesc =
      module->getFileSystem().getFileDescriptor(fd);
    storage::Stat statNative = fileDesc.stat();

    if (statNative.failed) {
        SPDLOG_ERROR("Failed stat: {}", statNative.wasiErrno);
        return statNative.wasiErrno;
    }

    statApp->fs_filetype = statNative.wasiFiletype;
    statApp->fs_rights_base = fileDesc.getActualRightsBase();
    statApp->fs_rights_inheriting = fileDesc.getActualRightsInheriting();

    // TODO - set fs flags
    statApp->fs_flags = 0;

    return __WASI_ESUCCESS;
}

static int32_t wasi_fd_fdstat_set_flags(wasm_exec_env_t exec_env,
                                        int32_t a,
                                        int32_t b)
{
    SPDLOG_DEBUG("S - fd_fdstat_set_flags");
    throw std::runtime_error("fd_fdstat_set_flags not implemented");
    return 0;
}

static int32_t doFileStat(int32_t fd,
                          const std::string& relativePath,
                          __wasi_filestat_t* statApp)
{
    WAMRWasmModule* module = getExecutingWAMRModule();
    storage::FileDescriptor& fileDesc =
      module->getFileSystem().getFileDescriptor(fd);
    storage::Stat statNative = fileDesc.stat(relativePath);
    if (statNative.failed) {
        SPDLOG_ERROR("Failed stat: {}", statNative.wasiErrno);
        return statNative.wasiErrno;
    }

    VALIDATE_NATIVE_ADDR(statApp, sizeof(__wasi_filestat_t));
    statApp->st_dev = statNative.st_dev;
    statApp->st_ino = statNative.st_ino;
    statApp->st_filetype = statNative.wasiFiletype;
    statApp->st_nlink = statNative.st_nlink;
    statApp->st_size = statNative.st_size;
    statApp->st_atim = statNative.st_atim;
    statApp->st_mtim = statNative.st_mtim;
    statApp->st_ctim = statNative.st_ctim;

    return __WASI_ESUCCESS;
}

static int32_t wasi_fd_filestat_get(wasm_exec_env_t exec_env,
                                    int32_t fd,
                                    __wasi_filestat_t* statApp)
{
    SPDLOG_DEBUG("S - fd_filestat_get {}", fd);

    return doFileStat(fd, "", statApp);
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
    const std::string& pathStr = fileDesc.getPath();

    VALIDATE_NATIVE_ADDR(path, pathStr.size());
    std::copy(pathStr.begin(), pathStr.end(), path);

    return __WASI_ESUCCESS;
}

typedef struct wasi_prestat_app
{
    __wasi_preopentype_t pr_type;
    uint32_t pr_name_len;
} wasi_prestat_app_t;

static int32_t wasi_fd_prestat_get(wasm_exec_env_t exec_env,
                                   int32_t fd,
                                   wasi_prestat_app_t* prestatApp)
{
    SPDLOG_DEBUG("S - fd_prestat_get {}", fd);

    WAMRWasmModule* module = getExecutingWAMRModule();
    if (!module->getFileSystem().fileDescriptorExists(fd)) {
        return __WASI_EBADF;
    }

    VALIDATE_NATIVE_ADDR(prestatApp, sizeof(wasi_prestat_app_t));

    storage::FileDescriptor& fileDesc =
      module->getFileSystem().getFileDescriptor(fd);

    prestatApp->pr_type = fileDesc.wasiPreopenType;
    prestatApp->pr_name_len = fileDesc.getPath().size();

    return __WASI_ESUCCESS;
}

typedef struct iovec_app
{
    uint32_t buffOffset;
    uint32_t buffLen;
} iovec_app_t;

static int32_t wasi_fd_read(wasm_exec_env_t exec_env,
                            int32_t fd,
                            iovec_app_t* ioVecBuffApp,
                            int32_t ioVecCountApp,
                            int32_t* bytesRead)
{
    WAMRWasmModule* module = getExecutingWAMRModule();
    storage::FileSystem& fileSystem = module->getFileSystem();
    std::string path = fileSystem.getPathForFd(fd);

    SPDLOG_DEBUG("S - fd_read {} ({})", fd, path);

    storage::FileDescriptor fileDesc = fileSystem.getFileDescriptor(fd);

    // Translate app iovecs to native ones
    auto ioVecBuffNative = new ::iovec[ioVecCountApp];
    for (int i = 0; i < ioVecCountApp; i++) {
        VALIDATE_APP_ADDR(ioVecBuffApp[i].buffOffset,
                          sizeof(char) * ioVecBuffApp[i].buffLen);

        ::iovec ioVecNative{
            .iov_base = ADDR_APP_TO_NATIVE(ioVecBuffApp[i].buffOffset),
            .iov_len = ioVecBuffApp[i].buffLen,
        };

        ioVecBuffNative[i] = ioVecNative;
    }

    // Read from fd
    VALIDATE_NATIVE_ADDR(bytesRead, sizeof(int32_t))
    *bytesRead = ::readv(fileDesc.getLinuxFd(), ioVecBuffNative, ioVecCountApp);

    delete[] ioVecBuffNative;

    return __WASI_ESUCCESS;
}

static int32_t wasi_fd_readdir(wasm_exec_env_t exec_env,
                               int32_t a,
                               int32_t* b,
                               char* c,
                               int64_t d,
                               int32_t e)
{
    SPDLOG_DEBUG("S - fd_readdir");
    throw std::runtime_error("fd_readdir not implemented");
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
    VALIDATE_NATIVE_ADDR(newOffset, sizeof(__wasi_filesize_t));

    storage::FileDescriptor& fileDesc =
      module->getFileSystem().getFileDescriptor(fd);
    auto wasiErrno = fileDesc.seek(offset, whence, newOffset);

    return wasiErrno;
}

static int32_t wasi_fd_write(wasm_exec_env_t exec_env,
                             int32_t fd,
                             iovec_app_t* ioVecBuffApp,
                             int32_t ioVecCountApp,
                             int32_t* bytesWritten)
{
    WAMRWasmModule* module = getExecutingWAMRModule();
    storage::FileSystem& fileSystem = module->getFileSystem();
    std::string path = fileSystem.getPathForFd(fd);

    SPDLOG_DEBUG("S - fd_write {} ({})", fd, path);

    storage::FileDescriptor& fileDesc = fileSystem.getFileDescriptor(fd);

    // Translate the app iovecs into native iovecs
    VALIDATE_NATIVE_ADDR(reinterpret_cast<void*>(ioVecBuffApp),
                         sizeof(iovec_app_t) * ioVecCountApp)
    auto ioVecBuffNative = new ::iovec[ioVecCountApp];
    for (int i = 0; i < ioVecCountApp; i++) {
        VALIDATE_APP_ADDR(ioVecBuffApp[i].buffOffset,
                          sizeof(char) * ioVecBuffApp[i].buffLen);

        ::iovec ioVecNative{
            .iov_base = ADDR_APP_TO_NATIVE(ioVecBuffApp[i].buffOffset),
            .iov_len = ioVecBuffApp[i].buffLen,
        };

        ioVecBuffNative[i] = ioVecNative;
    }

    // Write to fd
    VALIDATE_NATIVE_ADDR(bytesWritten, sizeof(int32_t))
    *bytesWritten =
      ::writev(fileDesc.getLinuxFd(), ioVecBuffNative, ioVecCountApp);
    if (*bytesWritten < 0) {
        SPDLOG_ERROR(
          "writev failed on fd {}: {}", fileDesc.getLinuxFd(), strerror(errno));
    }

    // Capture stdout if needed
    conf::FaasmConfig& conf = conf::getFaasmConfig();
    bool isStd = fd <= 2;
    if (isStd && conf.captureStdout == "on") {
        module->captureStdout(ioVecBuffNative, ioVecCountApp);
    }

    delete[] ioVecBuffNative;

    return __WASI_ESUCCESS;
}

static int32_t wasi_path_create_directory(wasm_exec_env_t exec_env,
                                          int32_t a,
                                          int32_t* b,
                                          char* c)
{
    SPDLOG_DEBUG("S - path_create_directory");
    throw std::runtime_error("path_create_directory not implemented");
    return 0;
}

static int32_t wasi_path_filestat_get(wasm_exec_env_t exec_env,
                                      int32_t fd,
                                      int32_t lookupFlags,
                                      char* path,
                                      int32_t pathLen,
                                      __wasi_filestat_t* statApp)
{
    WAMRWasmModule* module = getExecutingWAMRModule();
    VALIDATE_NATIVE_ADDR(path, pathLen);
    const std::string pathStr(path, pathLen);

    SPDLOG_DEBUG("S - path_filestat_get {} ({})", fd, pathStr);

    return doFileStat(fd, pathStr, statApp);
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
    SPDLOG_DEBUG("S - path_link");
    throw std::runtime_error("path_link not implemented");
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
                              int32_t* fdApp)
{
    WAMRWasmModule* module = getExecutingWAMRModule();

    VALIDATE_NATIVE_ADDR(path, pathLen);
    const std::string pathStr(path, pathLen);

    SPDLOG_DEBUG("S - path_open {} {} {}", fdNative, pathStr, pathLen);

    VALIDATE_NATIVE_ADDR(fdApp, sizeof(int32_t))
    *fdApp = module->getFileSystem().openFileDescriptor(fdNative,
                                                        pathStr,
                                                        rightsBase,
                                                        rightsInheriting,
                                                        lookupFlags,
                                                        openFlags,
                                                        fdFlags);

    if (*fdApp < 0) {
        return -1 * *fdApp;
    }

    return __WASI_ESUCCESS;
}

static int32_t wasi_path_remove_directory(wasm_exec_env_t exec_env,
                                          int32_t a,
                                          int32_t* b,
                                          char* c)
{
    SPDLOG_DEBUG("S - path_remove_directory");
    throw std::runtime_error("path_remove_directory not implemented");
    return 0;
}

static int32_t wasi_path_rename(wasm_exec_env_t exec_env,
                                int32_t a,
                                int32_t* b,
                                char* c,
                                int32_t d,
                                int32_t* e,
                                char* f)
{
    SPDLOG_DEBUG("S - path_rename");
    throw std::runtime_error("path_rename not implemented");
    return 0;
}

static int32_t wasi_path_unlink_file(wasm_exec_env_t exec_env,
                                     int32_t a,
                                     int32_t* b,
                                     char* c)
{
    SPDLOG_DEBUG("S - path_unlink_file");
    throw std::runtime_error("path_unlink_file not implemented");
    return 0;
}

static NativeSymbol wasiNs[] = {
    REG_WASI_NATIVE_FUNC(fd_close, "(i)i"),
    REG_WASI_NATIVE_FUNC(fd_fdstat_get, "(i*)i"),
    REG_WASI_NATIVE_FUNC(fd_fdstat_set_flags, "(ii)i"),
    REG_WASI_NATIVE_FUNC(fd_filestat_get, "(i*)i"),
    REG_WASI_NATIVE_FUNC(fd_prestat_dir_name, "(i*~)i"),
    REG_WASI_NATIVE_FUNC(fd_prestat_get, "(i*)i"),
    REG_WASI_NATIVE_FUNC(fd_read, "(i*i*)i"),
    REG_WASI_NATIVE_FUNC(fd_readdir, "(i*~I*)i"),
    REG_WASI_NATIVE_FUNC(fd_seek, "(iIi*)i"),
    REG_WASI_NATIVE_FUNC(fd_write, "(i*i*)i"),
    REG_WASI_NATIVE_FUNC(path_create_directory, "(i*~)i"),
    REG_WASI_NATIVE_FUNC(path_filestat_get, "(ii*~*)i"),
    REG_WASI_NATIVE_FUNC(path_link, "(ii*~i*~)i"),
    REG_WASI_NATIVE_FUNC(path_open, "(ii*~iIIi*)i"),
    REG_WASI_NATIVE_FUNC(path_remove_directory, "(i*~)i"),
    REG_WASI_NATIVE_FUNC(path_rename, "(i*~i*~)i"),
    REG_WASI_NATIVE_FUNC(path_unlink_file, "(i*~)i"),
};

uint32_t getFaasmWasiFilesystemApi(NativeSymbol** nativeSymbols)
{
    *nativeSymbols = wasiNs;
    return sizeof(wasiNs) / sizeof(NativeSymbol);
}
}
