#include <enclave/inside/EnclaveWasmModule.h>
#include <enclave/inside/native.h>
#include <enclave/inside/ocalls.h>
#include <wamr/types.h>
#include <wamr/WAMRModuleMixin.h>
#include <wasm/WasmEnvironment.h>

#include <string>

#include <wasm_export.h>

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

// For fd_fdstat_get we need to:
// 1. Validate the return pointer provided by the address
// 2. Retrieve the stat for the corresponding file descriptor
// 3. Copy the results into the provided pointer
// We do steps 1 and 3 inside the enclave, and do an OCall for step 2
static int32_t wasi_fd_fdstat_get(wasm_exec_env_t execEnv,
                                  int32_t fd,
                                  __wasi_fdstat_t* statWasm)
{
    SPDLOG_DEBUG_SGX("S - wasi_fd_fdstat_get %i", fd);
    GET_EXECUTING_MODULE_AND_CHECK(execEnv);

    if (!module->validateNativePointer(statWasm, sizeof(__wasi_fdstat_t)))
    {
        SPDLOG_ERROR_SGX("Pointer provided by WASM is not a valid pointer");
        return __WASI_EBADF;
    }

    uint8_t wasiFileType;
    uint64_t rightsBase;
    uint64_t rightsInheriting;
    int32_t returnValue;
    sgx_status_t sgxReturnValue;
    if ((sgxReturnValue = ocallWasiFdFdstatGet(&returnValue,
                                                    fd,
                                                    &wasiFileType,
                                                    &rightsBase,
                                                    &rightsInheriting)) != SGX_SUCCESS) {
        SET_ERROR(FAASM_SGX_OCALL_ERROR(sgxReturnValue));
    }

    statWasm->fs_filetype = wasiFileType;
    statWasm->fs_rights_base = rightsBase;
    statWasm->fs_rights_inheriting = rightsInheriting;
    statWasm->fs_flags = 0;

    return returnValue;
}

static int32_t wasi_fd_fdstat_set_flags(wasm_exec_env_t exec_env,
                                        int32_t a,
                                        int32_t b)
{
    SPDLOG_DEBUG_SGX("S - fd_fdstat_set_flags");
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

// To run fd_prestat_get we need to:
// 1. Validate the pointer passed by the WASM application
// 2. Get the information for the file descriptor
// We do step 1 inside the enclave, and do an OCall for step 2
static int32_t wasi_fd_prestat_get(wasm_exec_env_t execEnv,
                                   int32_t fd,
                                   wasi_prestat_app_t* prestatWasm)
{
    SPDLOG_DEBUG_SGX("S - wasi_fd_prestat_get");
    GET_EXECUTING_MODULE_AND_CHECK(execEnv);

    if (!module->validateNativePointer(prestatWasm, sizeof(wasi_prestat_app_t)))
    {
        SPDLOG_ERROR_SGX("Pointer provided by WASM is not a valid pointer");
        return __WASI_EBADF;
    }

    int returnValue;
    sgx_status_t sgxReturnValue;
    if ((sgxReturnValue = ocallWasiFdPrestatGet(&returnValue,
                                                fd,
                                                &(prestatWasm->pr_type),
                                                &(prestatWasm->pr_name_len))) != SGX_SUCCESS) {
        SET_ERROR(FAASM_SGX_OCALL_ERROR(sgxReturnValue));
    }

    return returnValue;
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

// To implement fd_write we need to:
// 1. Validate the wasm pointers
// 2. Translate the wasm iovecs into native iovecs
// 3. Do the actual writing to the fd
// 4. Return the bytes written by populating the return pointer
// We do steps 1 and 2 inside the enclave, and we do an OCall for step 3
static int wasi_fd_write(wasm_exec_env_t execEnv,
                         int32_t fd,
                         iovec_app_t* ioVecBuffWasm,
                         int32_t ioVecCountWasm,
                         int32_t* bytesWritten)
{
    SPDLOG_DEBUG_SGX("S - wasi_fd_write %i", fd);
    GET_EXECUTING_MODULE_AND_CHECK(execEnv);

    // Check pointers
    module->validateNativePointer(reinterpret_cast<void*>(ioVecBuffWasm),
                                  sizeof(iovec_app_t) * ioVecCountWasm);
    module->validateNativePointer(bytesWritten, sizeof(int32_t));

    // Translate the wasm iovecs into native iovecs and serialise to transfer
    // as an OCall
    uint8_t* ioVecBases[ioVecCountWasm];
    size_t ioVecLens[ioVecCountWasm];
    for (int i = 0; i < ioVecCountWasm; i++) {
        module->validateWasmOffset(ioVecBuffWasm[i].buffOffset,
                                   sizeof(char) * ioVecBuffWasm[i].buffLen);

        ioVecBases[i] = module->wamrWasmPointerToNative(ioVecBuffWasm[i].buffOffset);
        ioVecLens[i] = ioVecBuffWasm[i].buffLen;
    }

    return ocallWasiFdWrite(fd,
                            ioVecBases,
                            ioVecCountWasm * sizeof(uint8_t*),
                            ioVecLens,
                            ioVecCountWasm * sizeof(size_t),
                            ioVecCountWasm,
                            bytesWritten);
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
