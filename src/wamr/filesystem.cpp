#include <faabric/util/logging.h>
#include <wamr/native.h>

#include <stdexcept>
#include <wasm_export.h>

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

static int32_t wasi_fd_close(wasm_exec_env_t exec_env, int32_t a)
{
    SPDLOG_DEBUG("S - fd_close");
    throw std::runtime_error("fd_close not implemented");
    return 0;
}

static int32_t wasi_fd_fdstat_get(wasm_exec_env_t exec_env,
                                  int32_t a,
                                  int32_t* b)
{
    SPDLOG_DEBUG("S - fd_fdstat_get");
    throw std::runtime_error("fd_fdstat_get not implemented");
    return 0;
}

static int32_t wasi_fd_fdstat_set_flags(wasm_exec_env_t exec_env,
                                        int32_t a,
                                        int32_t b)
{
    SPDLOG_DEBUG("S - fd_fdstat_set_flags");
    throw std::runtime_error("fd_fdstat_set_flags not implemented");
    return 0;
}

static int32_t wasi_fd_filestat_get(wasm_exec_env_t exec_env,
                                    int32_t a,
                                    int32_t* b)
{
    SPDLOG_DEBUG("S - fd_filestat_get");
    throw std::runtime_error("fd_filestat_get not implemented");
    return 0;
}

static int32_t wasi_fd_prestat_dir_name(wasm_exec_env_t exec_env,
                                        int32_t a,
                                        int32_t* b,
                                        char* c)
{
    SPDLOG_DEBUG("S - fd_prestat_dir_name");
    throw std::runtime_error("fd_prestat_dir_name not implemented");
    return 0;
}

static int32_t wasi_fd_prestat_get(wasm_exec_env_t exec_env,
                                   int32_t a,
                                   int32_t* b)
{
    SPDLOG_DEBUG("S - fd_prestat_get");
    throw std::runtime_error("fd_prestat_get not implemented");
    return 0;
}

static int32_t wasi_fd_read(wasm_exec_env_t exec_env,
                            int32_t a,
                            int32_t* b,
                            int32_t c,
                            int32_t* d)
{
    SPDLOG_DEBUG("S - fd_read");
    throw std::runtime_error("fd_read not implemented");
    return 0;
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
                            int32_t a,
                            int64_t b,
                            int32_t c,
                            int32_t* d)
{
    SPDLOG_DEBUG("S - fd_seek");
    throw std::runtime_error("fd_seek not implemented");
    return 0;
}

static int32_t wasi_fd_write(wasm_exec_env_t exec_env,
                             int32_t a,
                             int32_t* b,
                             int32_t c,
                             int32_t* d)
{
    SPDLOG_DEBUG("S - fd_write");
    throw std::runtime_error("fd_write not implemented");
    return 0;
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
                                      int32_t a,
                                      int32_t b,
                                      int32_t* c,
                                      char* d,
                                      int32_t* e)
{
    SPDLOG_DEBUG("S - path_filestat_get");
    throw std::runtime_error("path_filestat_get not implemented");
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
    SPDLOG_DEBUG("S - path_link_wrapper");
    throw std::runtime_error("path_link_wrapper not implemented");
    return 0;
}

static int32_t wasi_path_open(wasm_exec_env_t exec_env,
                              int32_t a,
                              int32_t b,
                              int32_t* c,
                              char* d,
                              int32_t e,
                              int64_t f,
                              int64_t g,
                              int32_t h,
                              int32_t* i)
{
    SPDLOG_DEBUG("S - path_open_wrapper");
    throw std::runtime_error("path_open_wrapper not implemented");
    return 0;
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
    throw std::runtime_error("path_rename_wrapper not implemented");
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
