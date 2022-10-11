#include <enclave/inside/native.h>
#include <enclave/inside/ocalls.h>
#include <enclave/inside/types.h>
#include <wasm/WasmEnvironment.h>

#include <string>

#include <wasm_export.h>
#include <wasmtime_ssp.h>

namespace sgx {
uint32_t doWasiDup(uint32_t fd)
{
    return 0;
}

static uint32_t __wasi_fd_dup_wrapper(wasm_exec_env_t exec_env,
                                      uint32_t fd,
                                      uint32_t* resFd)
{
    SPDLOG_DEBUG_SGX("S - __wasi_fd_dup");
    return 0;
}

static uint32_t dup_wrapper(wasm_exec_env_t exec_env, uint32_t fd)
{
    SPDLOG_DEBUG_SGX("S - dup");
    return 0;
}

static uint32_t getcwd_wrapper(wasm_exec_env_t exec_env,
                               char* buf,
                               uint32_t bufLen)
{
    SPDLOG_DEBUG_SGX("S - getcwd");

    // Fake working directory
    strcpy(buf, FAKE_WORKING_DIR);

    return __WASI_ESUCCESS;
}

static uint32_t getpwnam_wrapper(wasm_exec_env_t exec_env, uint32_t a)
{
    SPDLOG_DEBUG_SGX("S - getpwnam");
    throw std::runtime_error("getpwnam not implemented");
}

static int32_t sendfile_wrapper(wasm_exec_env_t exec_env,
                                int32_t out_fd,
                                int32_t in_fd,
                                int32_t offset,
                                int32_t count)
{
    SPDLOG_DEBUG_SGX("S - sendfile {}");
    throw std::runtime_error("sendfile not implemented");
}

static int32_t tempnam_wrapper(wasm_exec_env_t exec_env, int32_t a, int32_t b)
{
    SPDLOG_DEBUG_SGX("S - tempnam");
    return 0;
}

static NativeSymbol ns[] = {
    REG_NATIVE_FUNC(__wasi_fd_dup, "(i*)i"), REG_NATIVE_FUNC(dup, "(i)i"),
    REG_NATIVE_FUNC(getcwd, "(*~)i"),        REG_NATIVE_FUNC(getpwnam, "(i)i"),
    REG_NATIVE_FUNC(sendfile, "(iiii)i"),    REG_NATIVE_FUNC(tempnam, "(ii)i"),
};

uint32_t getFaasmFilesystemApi(NativeSymbol** nativeSymbols)
{
    *nativeSymbols = ns;
    return sizeof(ns) / sizeof(NativeSymbol);
}

// ---------- WASI symbols ----------

static int32_t wasi_fd_close(wasm_exec_env_t execEnv, int32_t fd)
{
    SPDLOG_DEBUG_SGX("S - wasi_fd_close %i", fd);
    return 0;
}

static int32_t wasi_fd_fdstat_get(wasm_exec_env_t execEnv,
                                  int32_t fd,
                                  __wasi_fdstat_t* statWasm)
{
    SPDLOG_DEBUG_SGX("S - wasi_fd_fdstat_get %i", fd);
    return 0;
}

static int32_t wasi_fd_fdstat_set_flags(wasm_exec_env_t exec_env,
                                        int32_t a,
                                        int32_t b)
{
    SPDLOG_DEBUG_SGX("S - fd_fdstat_set_flags");
    return 0;
}

static int32_t doFileStat(uint32_t fd,
                          const std::string& relativePath,
                          __wasi_filestat_t* statWasm)
{
    return 0;
}

static int32_t wasi_fd_filestat_get(wasm_exec_env_t exec_env,
                                    int32_t fd,
                                    __wasi_filestat_t* statWasm)
{
    SPDLOG_DEBUG_SGX("S - fd_filestat_get");
    return 0;
}

static int32_t wasi_fd_prestat_dir_name(wasm_exec_env_t exec_env,
                                        int32_t fd,
                                        char* path,
                                        int32_t* pathLen)
{
    SPDLOG_DEBUG_SGX("S - fd_prestat_dir_name");
    return 0;
}

static int32_t wasi_fd_prestat_get(wasm_exec_env_t exec_env,
                                   int32_t fd,
                                   wasi_prestat_app_t* prestatWasm)
{
    SPDLOG_DEBUG_SGX("S - fd_prestat_get");
    return 0;
}

static int32_t wasi_fd_read(wasm_exec_env_t exec_env,
                            int32_t fd,
                            iovec_app_t* ioVecBuffWasm,
                            int32_t ioVecCountWasm,
                            int32_t* bytesRead)
{
    SPDLOG_DEBUG_SGX("S - fd_read");
    return 0;
}

static int32_t wasi_fd_readdir(wasm_exec_env_t exec_env,
                               int32_t a,
                               int32_t* b,
                               char* c,
                               int64_t d,
                               int32_t e)
{
    SPDLOG_DEBUG_SGX("S - fd_readdir");
    return 0;
}

static int wasi_fd_seek(wasm_exec_env_t execEnv, int a, int64 b, int c, int d)
{
    SPDLOG_DEBUG_SGX("S - wasi_fd_seek");
    return 0;
}

static int wasi_fd_write(wasm_exec_env_t execEnv,
                         int32_t fd,
                         iovec_app_t* ioVecBuffWasm,
                         int32_t ioVecCountWasm,
                         int32_t* bytesWritten)
{
    SPDLOG_DEBUG_SGX("S - wasi_fd_write %i", fd);
    return 0;
}

static int32_t wasi_path_create_directory(wasm_exec_env_t exec_env,
                                          int32_t a,
                                          int32_t* b,
                                          char* c)
{
    SPDLOG_DEBUG_SGX("S - path_create_directory");
    return 0;
}

static int32_t wasi_path_filestat_get(wasm_exec_env_t exec_env,
                                      int32_t fd,
                                      int32_t lookupFlags,
                                      char* path,
                                      int32_t pathLen,
                                      __wasi_filestat_t* statWasm)
{
    SPDLOG_DEBUG_SGX("S - path_filestat_get");
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
    SPDLOG_DEBUG_SGX("S - path_link");
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
    SPDLOG_DEBUG_SGX("S - wasi_path_open");
    return 0;
}

static int32_t wasi_path_readlink(wasm_exec_env_t exec_env,
                                  uint32_t fd,
                                  const char* path,
                                  uint32_t pathLen,
                                  char* buf,
                                  uint32_t bufLen,
                                  uint32_t* bufApp)
{
    SPDLOG_DEBUG_SGX("S - path_readlink");
    return 0;
}

static int32_t wasi_path_remove_directory(wasm_exec_env_t exec_env,
                                          int32_t a,
                                          int32_t* b,
                                          char* c)
{
    SPDLOG_DEBUG_SGX("S - path_remove_directory");
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
    SPDLOG_DEBUG_SGX("S - path_rename");
    return 0;
}

static int32_t wasi_path_symlink(wasm_exec_env_t exec_env,
                                 const char* oldPath,
                                 uint32_t oldPathLen,
                                 uint32_t fd,
                                 const char* newPath,
                                 uint32_t newPathLen)
{
    SPDLOG_DEBUG_SGX("S - path_symlink");
    return 0;
}

static int32_t wasi_path_unlink_file(wasm_exec_env_t exec_env,
                                     uint32_t fd,
                                     char* path,
                                     uint32_t pathLen)
{
    SPDLOG_DEBUG_SGX("S - path_unlink_file");
    return 0;
}

static NativeSymbol wasiNs[] = {
    REG_WASI_NATIVE_FUNC(fd_close, "(i)i"),
    REG_WASI_NATIVE_FUNC(fd_fdstat_get, "(ii)i"),
    REG_WASI_NATIVE_FUNC(fd_fdstat_set_flags, "(ii)i"),
    REG_WASI_NATIVE_FUNC(fd_filestat_get, "(i*)i"),
    REG_WASI_NATIVE_FUNC(fd_prestat_dir_name, "(i*~)i"),
    REG_WASI_NATIVE_FUNC(fd_prestat_get, "(i*)i"),
    REG_WASI_NATIVE_FUNC(fd_read, "(i*i*)i"),
    REG_WASI_NATIVE_FUNC(fd_readdir, "(i*~I*)i"),
    REG_WASI_NATIVE_FUNC(fd_seek, "(iIii)i"),
    REG_WASI_NATIVE_FUNC(fd_write, "(iiii)i"),
    REG_WASI_NATIVE_FUNC(path_create_directory, "(i*~)i"),
    REG_WASI_NATIVE_FUNC(path_filestat_get, "(ii*~*)i"),
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
