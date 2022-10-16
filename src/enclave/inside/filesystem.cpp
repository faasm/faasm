#include <enclave/inside/EnclaveWasmModule.h>
#include <enclave/inside/native.h>
#include <enclave/inside/ocalls.h>
#include <wamr/WAMRModuleMixin.h>
#include <wamr/types.h>
#include <wasm/WasmEnvironment.h>

#include <string>

#include <wasm_export.h>

// We don't have access to linux/limits.h inside the enclave
#define PATH_MAX 4096

namespace sgx {
static uint32_t __wasi_fd_dup_wrapper(wasm_exec_env_t execEnv,
                                      uint32_t fd,
                                      uint32_t* resFd)
{
    SPDLOG_DEBUG_SGX("S - __wasi_fd_dup %i", fd);
    GET_EXECUTING_MODULE_AND_CHECK(execEnv);

    uint32_t returnValue;
    sgx_status_t sgxReturnValue;
    if ((sgxReturnValue = ocallDup(&returnValue, fd)) != SGX_SUCCESS) {
        SET_ERROR(FAASM_SGX_OCALL_ERROR(sgxReturnValue));
    }

    *resFd = returnValue;

    return __WASI_ESUCCESS;
}

static uint32_t dup_wrapper(wasm_exec_env_t execEnv, uint32_t fd)
{
    SPDLOG_DEBUG_SGX("S - dup %i", fd);

    uint32_t returnValue;
    sgx_status_t sgxReturnValue;
    if ((sgxReturnValue = ocallDup(&returnValue, fd)) != SGX_SUCCESS) {
        SET_ERROR(FAASM_SGX_OCALL_ERROR(sgxReturnValue));
    }

    return returnValue;
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
    throw std::runtime_error("sendfile not implemented");
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

static uint32_t wasi_fd_allocate(wasm_exec_env_t exec_env,
                                 __wasi_fd_t fd,
                                 __wasi_filesize_t offset,
                                 __wasi_filesize_t len)
{
    SPDLOG_DEBUG_SGX("wasi_fd_allocate %i", fd);
    throw std::runtime_error("wasi_fd_allocate not implemented");
}

static int32_t wasi_fd_close(wasm_exec_env_t execEnv, int32_t fd)
{
    // We ignore closing for now
    SPDLOG_DEBUG_SGX("S - wasi_fd_close %i", fd);

    return __WASI_ESUCCESS;
    ;
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

    module->validateNativePointer(statWasm, sizeof(__wasi_fdstat_t));

    uint8_t wasiFileType;
    uint64_t rightsBase;
    uint64_t rightsInheriting;
    int32_t returnValue;
    sgx_status_t sgxReturnValue;
    if ((sgxReturnValue = ocallWasiFdFdstatGet(
           &returnValue, fd, &wasiFileType, &rightsBase, &rightsInheriting)) !=
        SGX_SUCCESS) {
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
    throw std::runtime_error("fd_fdstat_set_flags not implemented");
}

// To run fd_filestat_get we need to:
// 1. Get the stat from the filedescriptors path
// 2. Copy the contents into WASM memory
// We do step 1 in an OCall and step 2 inside the enclave
static int32_t wasi_fd_filestat_get(wasm_exec_env_t execEnv,
                                    int32_t fd,
                                    __wasi_filestat_t* statWasm)
{
    SPDLOG_DEBUG_SGX("S - wasi_fd_filestat_get %i", fd);
    GET_EXECUTING_MODULE_AND_CHECK(execEnv);
    module->validateNativePointer(statWasm, sizeof(__wasi_filestat_t));

    int32_t returnValue;
    sgx_status_t sgxReturnValue;
    if ((sgxReturnValue = ocallWasiFdFilestatGet(&returnValue,
                                                 fd,
                                                 &(statWasm->st_filetype),
                                                 &(statWasm->st_dev),
                                                 &(statWasm->st_ino),
                                                 &(statWasm->st_nlink),
                                                 &(statWasm->st_size),
                                                 &(statWasm->st_atim),
                                                 &(statWasm->st_mtim),
                                                 &(statWasm->st_ctim))) !=
        SGX_SUCCESS) {
        SET_ERROR(FAASM_SGX_OCALL_ERROR(sgxReturnValue));
    }

    return returnValue;
}

static uint32_t wasi_fd_pread(wasm_exec_env_t exec_env,
                              __wasi_fd_t fd,
                              iovec_app_t* iovecWasm,
                              uint32_t iovecLen,
                              __wasi_filesize_t offset,
                              uint32_t* nReadWasm)
{
    SPDLOG_DEBUG_SGX("S - fd_pread %i", fd);
    throw std::runtime_error("fd_pread not implemented");
}

// To run fd_prestat_dir_name we need to:
// 1. Make sure that the file descriptor exists
// 2. Get the path for the corresponding file descriptor
// 3. Copy the path to the provided variables
// We do steps 1 and 2 in an OCall, and do step 3 inside the enclave
static int32_t wasi_fd_prestat_dir_name(wasm_exec_env_t execEnv,
                                        int32_t fd,
                                        char* path,
                                        int32_t pathLen)
{
    SPDLOG_DEBUG_SGX("S - wasi_fd_prestat_dir_name %i", fd);
    GET_EXECUTING_MODULE_AND_CHECK(execEnv);
    module->validateNativePointer(path, pathLen);

    int returnValue;
    sgx_status_t sgxReturnValue;
    char retPath[pathLen];
    if ((sgxReturnValue = ocallWasiFdPrestatDirName(
           &returnValue, fd, retPath, pathLen)) != SGX_SUCCESS) {
        SET_ERROR(FAASM_SGX_OCALL_ERROR(sgxReturnValue));
    }

    module->writeStringToWasmMemory(std::string(retPath), path);

    return __WASI_ESUCCESS;
}

// To run fd_prestat_get we need to:
// 1. Validate the pointer passed by the WASM application
// 2. Get the information for the file descriptor
// We do step 1 inside the enclave, and do an OCall for step 2
static int32_t wasi_fd_prestat_get(wasm_exec_env_t execEnv,
                                   int32_t fd,
                                   wasi_prestat_app_t* prestatWasm)
{
    SPDLOG_DEBUG_SGX("S - wasi_fd_prestat_get %i", fd);
    GET_EXECUTING_MODULE_AND_CHECK(execEnv);

    module->validateNativePointer(prestatWasm, sizeof(wasi_prestat_app_t));

    int returnValue;
    sgx_status_t sgxReturnValue;
    if ((sgxReturnValue = ocallWasiFdPrestatGet(&returnValue,
                                                fd,
                                                &(prestatWasm->pr_type),
                                                &(prestatWasm->pr_name_len))) !=
        SGX_SUCCESS) {
        SET_ERROR(FAASM_SGX_OCALL_ERROR(sgxReturnValue));
    }

    return returnValue;
}

static uint32_t wasi_fd_pwrite(wasm_exec_env_t exec_env,
                               __wasi_fd_t fd,
                               const iovec_app_t* iovecWasm,
                               uint32_t iovecLen,
                               __wasi_filesize_t offset,
                               uint32_t* nWrittenWasm)
{
    SPDLOG_DEBUG_SGX("S - fd_pwrite {}");
    throw std::runtime_error("fd_pwrite not implemented");
}

// To implement fd_read we need to:
// 1. Validate the wasm pointers
// 2. Translate the wasm iovecs into native iovecs
// 3. Do the actual reading from the fd
// 4. Return the bytes read by populating the return pointer
// We do steps 1 and 2 inside the enclave, and we do an OCall for step 3
static int32_t wasi_fd_read(wasm_exec_env_t execEnv,
                            int32_t fd,
                            iovec_app_t* ioVecBuffWasm,
                            int32_t ioVecCountWasm,
                            int32_t* bytesRead)
{
    SPDLOG_DEBUG_SGX("S - wasi_fd_read %i", fd);
    GET_EXECUTING_MODULE_AND_CHECK(execEnv);

    // Check pointers
    module->validateNativePointer(reinterpret_cast<void*>(ioVecBuffWasm),
                                  sizeof(iovec_app_t) * ioVecCountWasm);
    module->validateNativePointer(bytesRead, sizeof(int32_t));

    // Translate the wasm iovecs into native iovecs and serialise to transfer
    // as an OCall. Here we use that an iovec_app_t is a struct with a
    // uint8_t pointer and a size_t length
    uint8_t* ioVecBases[ioVecCountWasm];
    size_t ioVecLens[ioVecCountWasm];
    for (int i = 0; i < ioVecCountWasm; i++) {
        module->validateWasmOffset(ioVecBuffWasm[i].buffOffset,
                                   sizeof(char) * ioVecBuffWasm[i].buffLen);

        ioVecBases[i] =
          module->wamrWasmPointerToNative(ioVecBuffWasm[i].buffOffset);
        ioVecLens[i] = ioVecBuffWasm[i].buffLen;
    }

    int returnValue;
    sgx_status_t sgxReturnValue;
    if ((sgxReturnValue = ocallWasiFdRead(&returnValue,
                                          fd,
                                          ioVecBases,
                                          ioVecCountWasm * sizeof(uint8_t*),
                                          ioVecLens,
                                          ioVecCountWasm * sizeof(size_t),
                                          ioVecCountWasm,
                                          bytesRead)) != SGX_SUCCESS) {
    }

    return returnValue;
}

static int32_t wasi_fd_readdir(wasm_exec_env_t exec_env,
                               int32_t a,
                               int32_t* b,
                               char* c,
                               int64_t d,
                               int32_t e)
{
    SPDLOG_DEBUG_SGX("S - fd_readdir");
    throw std::runtime_error("fd_readdir not implemented");
}

static int wasi_fd_seek(wasm_exec_env_t execEnv,
                        int32_t fd,
                        int64_t offset,
                        int32_t whence,
                        __wasi_filesize_t* newOffset)
{
    SPDLOG_DEBUG_SGX("S - wasi_fd_seek %i", fd);
    GET_EXECUTING_MODULE_AND_CHECK(execEnv);

    module->validateNativePointer(newOffset, sizeof(__wasi_filesize_t));

    int returnValue;
    sgx_status_t sgxReturnValue;
    if ((sgxReturnValue = ocallWasiFdSeek(
           &returnValue, fd, offset, whence, newOffset)) != SGX_SUCCESS) {
    }

    return returnValue;
}

static uint32_t wasi_fd_sync(wasm_exec_env_t exec_env, __wasi_fd_t fd)
{
    SPDLOG_DEBUG_SGX("S - wasi_fd_sync %i", fd);
    throw std::runtime_error("fd_sync not implemented");
}

static int32_t wasi_fd_tell(wasm_exec_env_t execEnv,
                            int32_t fd,
                            int32_t* resOffset)
{
    SPDLOG_DEBUG_SGX("S - wasi_fd_tell %i", fd);
    GET_EXECUTING_MODULE_AND_CHECK(execEnv);
    module->validateNativePointer(resOffset, sizeof(int32_t));

    int32_t returnValue;
    sgx_status_t sgxReturnValue;
    if ((sgxReturnValue = ocallWasiFdTell(&returnValue, fd, resOffset)) !=
        SGX_SUCCESS) {
    }

    return returnValue;
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
    // as an OCall. Here we use that an iovec_app_t is a struct with a
    // uint8_t pointer and a size_t length
    size_t totalBasesSize = 0;
    for (int i = 0; i < ioVecCountWasm; i++) {
        totalBasesSize += ioVecBuffWasm[i].buffLen;
    }

    uint8_t ioVecBases[totalBasesSize];
    // size_t ioVecLens[ioVecCountWasm];
    int32_t offset = 0;
    int32_t ioVecOffsets[ioVecCountWasm];
    for (int i = 0; i < ioVecCountWasm; i++) {
        module->validateWasmOffset(ioVecBuffWasm[i].buffOffset,
                                   sizeof(char) * ioVecBuffWasm[i].buffLen);

        memcpy(ioVecBases + offset,
               module->wamrWasmPointerToNative(ioVecBuffWasm[i].buffOffset),
               ioVecBuffWasm[i].buffLen);
        ioVecOffsets[i] = offset;
        offset += ioVecBuffWasm[i].buffLen;
    }

    int returnValue;
    sgx_status_t sgxReturnValue;
    if ((sgxReturnValue = ocallWasiFdWrite(&returnValue,
                                           fd,
                                           ioVecBases,
                                           totalBasesSize,
                                           ioVecOffsets,
                                           ioVecCountWasm,
                                           bytesWritten)) != SGX_SUCCESS) {
    }

    return returnValue;
}

static int32_t wasi_path_create_directory(wasm_exec_env_t exec_env,
                                          int32_t a,
                                          int32_t* b,
                                          char* c)
{
    SPDLOG_DEBUG_SGX("S - wasi_path_create_directory");
    throw std::runtime_error("wasi_path_create_directory not implemented");
}

// To run path_filestat_get we need to:
// 1. Get the stat from the path
// 2. Copy the contents into WASM memory
// We do step 1 in an OCall and step 2 inside the enclave
static int32_t wasi_path_filestat_get(wasm_exec_env_t execEnv,
                                      int32_t fd,
                                      int32_t lookupFlags,
                                      char* path,
                                      int32_t pathLen,
                                      __wasi_filestat_t* statWasm)
{
    GET_EXECUTING_MODULE_AND_CHECK(execEnv);
    module->validateNativePointer(path, pathLen);
    SPDLOG_DEBUG_SGX("S - wasi_path_filestat_get %i (%s)", fd, path);

    int32_t returnValue;
    sgx_status_t sgxReturnValue;
    if ((sgxReturnValue = ocallWasiPathFilestatGet(&returnValue,
                                                   fd,
                                                   path,
                                                   pathLen,
                                                   &(statWasm->st_filetype),
                                                   &(statWasm->st_dev),
                                                   &(statWasm->st_ino),
                                                   &(statWasm->st_nlink),
                                                   &(statWasm->st_size),
                                                   &(statWasm->st_atim),
                                                   &(statWasm->st_mtim),
                                                   &(statWasm->st_ctim))) !=
        SGX_SUCCESS) {
        SET_ERROR(FAASM_SGX_OCALL_ERROR(sgxReturnValue));
    }

    return returnValue;
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
    SPDLOG_DEBUG_SGX("wasi_path_filestat_set_times %i", fd);
    throw std::runtime_error("wasi_path_filestat_set_times not implemented");
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
    SPDLOG_DEBUG_SGX("S - wasi_path_link");
    throw std::runtime_error("wasi_path_link not implemented");
}

// To do path_open we need to:
// 1. Validate the provided pointers
// 2. Open the actual fd
// We do step 1 inside the enclave, and do an OCall for step 2
static int32_t wasi_path_open(wasm_exec_env_t execEnv,
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
    GET_EXECUTING_MODULE_AND_CHECK(execEnv);

    // Validate WASM pointers
    module->validateNativePointer(path, pathLen);
    module->validateNativePointer(fdWasm, sizeof(int32_t));
    SPDLOG_DEBUG_SGX("S - wasi_path_open %i (%s)", fdNative, path);

    int32_t returnValue;
    sgx_status_t sgxReturnValue;
    if ((sgxReturnValue = ocallWasiPathOpen(&returnValue,
                                            fdNative,
                                            lookupFlags,
                                            path,
                                            pathLen,
                                            openFlags,
                                            rightsBase,
                                            rightsInheriting,
                                            fdFlags,
                                            fdWasm)) != SGX_SUCCESS) {
        SET_ERROR(FAASM_SGX_OCALL_ERROR(sgxReturnValue));
    }

    return returnValue;
}

static int32_t wasi_path_readlink(wasm_exec_env_t execEnv,
                                  int32_t fd,
                                  char* path,
                                  int32_t pathLen,
                                  char* buf,
                                  int32_t bufLen,
                                  int32_t* resBytesUsed)
{
    GET_EXECUTING_MODULE_AND_CHECK(execEnv);

    // Validate WASM pointers
    module->validateNativePointer(path, pathLen);
    module->validateNativePointer(resBytesUsed, sizeof(int32_t));
    SPDLOG_DEBUG_SGX("S - wasi_path_readlink %i (%s)", fd, path);

    int32_t returnValue;
    sgx_status_t sgxReturnValue;
    if ((sgxReturnValue = ocallWasiPathReadlink(
           &returnValue, fd, path, pathLen, buf, bufLen, resBytesUsed)) !=
        SGX_SUCCESS) {
        SET_ERROR(FAASM_SGX_OCALL_ERROR(sgxReturnValue));
    }

    return returnValue;
}

static int32_t wasi_path_remove_directory(wasm_exec_env_t exec_env,
                                          int32_t a,
                                          int32_t* b,
                                          char* c)
{
    SPDLOG_DEBUG_SGX("S - path_remove_directory");
    throw std::runtime_error("path_remove_directory not implemented");
}

static int32_t wasi_path_rename(wasm_exec_env_t execEnv,
                                uint32_t oldFd,
                                char* oldPath,
                                uint32_t oldPathLen,
                                uint32_t newFd,
                                char* newPath,
                                uint32_t newPathLen)
{
    GET_EXECUTING_MODULE_AND_CHECK(execEnv);
    module->validateNativePointer(oldPath, oldPathLen);
    module->validateNativePointer(newPath, newPathLen);
    SPDLOG_DEBUG_SGX("S - wasi_path_rename %s (fd: %i) -> %s (fd: %i)",
                     oldPath,
                     oldFd,
                     newPath,
                     newFd);

    int32_t returnValue;
    sgx_status_t sgxReturnValue;
    if ((sgxReturnValue = ocallWasiPathRename(&returnValue,
                                              oldFd,
                                              oldPath,
                                              oldPathLen,
                                              newFd,
                                              newPath,
                                              newPathLen)) != SGX_SUCCESS) {
        SET_ERROR(FAASM_SGX_OCALL_ERROR(sgxReturnValue));
    }

    return returnValue;
}

static int32_t wasi_path_symlink(wasm_exec_env_t exec_env,
                                 const char* oldPath,
                                 uint32_t oldPathLen,
                                 uint32_t fd,
                                 const char* newPath,
                                 uint32_t newPathLen)
{
    SPDLOG_DEBUG_SGX("S - wasi_path_symlink");
    throw std::runtime_error("wasi_path_symlink not implemented");
}

static int32_t wasi_path_unlink_file(wasm_exec_env_t execEnv,
                                     uint32_t fd,
                                     char* path,
                                     uint32_t pathLen)
{
    SPDLOG_DEBUG_SGX("S - wasi_path_unlink_file %i", fd);
    GET_EXECUTING_MODULE_AND_CHECK(execEnv);
    module->validateNativePointer(path, pathLen);

    int32_t returnValue;
    sgx_status_t sgxReturnValue;
    if ((sgxReturnValue = ocallWasiPathUnlinkFile(
           &returnValue, fd, path, pathLen)) != SGX_SUCCESS) {
        SET_ERROR(FAASM_SGX_OCALL_ERROR(sgxReturnValue));
    }

    return returnValue;
}

static NativeSymbol wasiNs[] = {
    REG_WASI_NATIVE_FUNC(fd_allocate, "(iII)i"),
    REG_WASI_NATIVE_FUNC(fd_close, "(i)i"),
    REG_WASI_NATIVE_FUNC(fd_fdstat_get, "(ii)i"),
    REG_WASI_NATIVE_FUNC(fd_fdstat_set_flags, "(ii)i"),
    REG_WASI_NATIVE_FUNC(fd_filestat_get, "(i*)i"),
    REG_WASI_NATIVE_FUNC(fd_pread, "(i*iI*)i"),
    REG_WASI_NATIVE_FUNC(fd_prestat_dir_name, "(i*~)i"),
    REG_WASI_NATIVE_FUNC(fd_prestat_get, "(i*)i"),
    REG_WASI_NATIVE_FUNC(fd_read, "(i*i*)i"),
    REG_WASI_NATIVE_FUNC(fd_readdir, "(i*~I*)i"),
    REG_WASI_NATIVE_FUNC(fd_pwrite, "(i*iI*)i"),
    REG_WASI_NATIVE_FUNC(fd_seek, "(iIi*)i"),
    REG_WASI_NATIVE_FUNC(fd_sync, "(i)i"),
    REG_WASI_NATIVE_FUNC(fd_tell, "(i*)i"),
    REG_WASI_NATIVE_FUNC(fd_write, "(iiii)i"),
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
