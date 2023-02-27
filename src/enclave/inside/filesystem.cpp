#include <enclave/inside/native.h>

namespace sgx {
static int wasi_fd_close(wasm_exec_env_t execEnv, int a)
{
    return 0;
}

static int wasi_fd_seek(wasm_exec_env_t execEnv, int a, int64 b, int c, int d)
{
    return 0;
}

static int wasi_fd_write(wasm_exec_env_t execEnv, int a, int b, int c, int d)
{
    return 0;
}

static int wasi_fd_fdstat_get(wasm_exec_env_t execEnv, int a, int b)
{
    return 0;
}

static int32_t wasi_fd_fdstat_set_rights(wasm_exec_env_t exec_env,
                                         int32_t a,
                                         int64_t b,
                                         int64_t c)
{
    return 0;
}

static NativeSymbol wasiNs[] = {
    REG_WASI_NATIVE_FUNC(fd_close, "(i)i"),
    REG_WASI_NATIVE_FUNC(fd_seek, "(iIii)i"),
    REG_WASI_NATIVE_FUNC(fd_write, "(iiii)i"),
    REG_WASI_NATIVE_FUNC(fd_fdstat_get, "(ii)i"),
    REG_WASI_NATIVE_FUNC(fd_fdstat_set_rights, "(iII)i"),
};

uint32_t getFaasmWasiFilesystemApi(NativeSymbol** nativeSymbols)
{
    *nativeSymbols = wasiNs;
    return sizeof(wasiNs) / sizeof(NativeSymbol);
}
}
