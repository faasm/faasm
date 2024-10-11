#include <enclave/inside/EnclaveWasmModule.h>
#include <enclave/inside/native.h>
#include <wamr/WAMRModuleMixin.h>
#include <wamr/types.h>

namespace sgx {

// ---------------------------------------
// Non-WASI Filesystem calls
// ---------------------------------------

static int32_t __wasi_fd_dup_wrapper(wasm_exec_env_t execEnv,
                                     int32_t wasmFd,
                                     int32_t* resFd)
{
    SPDLOG_DEBUG_SGX("S - __wasi_fd_dup %i", wasmFd);
    GET_EXECUTING_MODULE_AND_CHECK(execEnv);

    int32_t returnValue;
    sgx_status_t sgxReturnValue;
    if ((sgxReturnValue = ocallWasiFdDup(&returnValue, wasmFd)) !=
        SGX_SUCCESS) {
        SET_ERROR(FAASM_SGX_OCALL_ERROR(sgxReturnValue));
    }

    *resFd = returnValue;

    return __WASI_ESUCCESS;
}

static int32_t dup_wrapper(wasm_exec_env_t execEnv, int32_t wasmFd)
{
    SPDLOG_DEBUG_SGX("S - dup %i", wasmFd);

    int32_t returnValue;
    sgx_status_t sgxReturnValue;
    if ((sgxReturnValue = ocallWasiFdDup(&returnValue, wasmFd)) !=
        SGX_SUCCESS) {
        SET_ERROR(FAASM_SGX_OCALL_ERROR(sgxReturnValue));
    }

    return returnValue;
}

static uint32_t getpwnam_wrapper(wasm_exec_env_t exec_env, uint32_t a)
{
    SPDLOG_DEBUG_SGX("S - getpwnam");

    UNIMPLEMENTED_WASM_INTRINSIC("getpwnam");
}

static int32_t sendfile_wrapper(wasm_exec_env_t exec_env,
                                int32_t out_fd,
                                int32_t in_fd,
                                int32_t offset,
                                int32_t count)
{
    SPDLOG_DEBUG_SGX("S - sendfile {}");

    UNIMPLEMENTED_WASM_INTRINSIC("sendfile");
}

static int32_t tempnam_wrapper(wasm_exec_env_t exec_env, int32_t a, int32_t b)
{
    SPDLOG_DEBUG_SGX("S - tempnam");

    UNIMPLEMENTED_WASM_INTRINSIC("tempnam");
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

// ---------------------------------------
// WASI Filesystem calls
// ---------------------------------------

static int wasi_fd_close(wasm_exec_env_t execEnv, int wasmFd)
{
    // We ignore closing for now
    SPDLOG_DEBUG_SGX("S - wasi_fd_close %i", wasmFd);

    return __WASI_ESUCCESS;
}

// To run fd_filestat_get we need to:
// 1. Get the stat from the filedescriptors path
// 2. Copy the contents into WASM memory
// We do step 1 in an OCall and step 2 inside the enclave
static int32_t wasi_fd_filestat_get(wasm_exec_env_t execEnv,
                                    int32_t wasmFd,
                                    __wasi_filestat_t* statWasm)
{
    SPDLOG_DEBUG_SGX("S - wasi_fd_filestat_get %i", wasmFd);
    GET_EXECUTING_MODULE_AND_CHECK(execEnv);
    module->validateNativePointer(statWasm, sizeof(__wasi_filestat_t));

    int32_t returnValue;
    sgx_status_t sgxReturnValue;
    if ((sgxReturnValue = ocallWasiFdFilestatGet(&returnValue,
                                                 wasmFd,
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

// To run fd_prestat_dir_name we need to:
// 1. Make sure that the file descriptor exists
// 2. Get the path for the corresponding file descriptor
// 3. Copy the path to the provided variables
// We do steps 1 and 2 in an OCall, and do step 3 inside the enclave
static int32_t wasi_fd_prestat_dir_name(wasm_exec_env_t execEnv,
                                        int32_t wasmFd,
                                        char* path,
                                        const int32_t pathLen)
{
    SPDLOG_DEBUG_SGX("S - wasi_fd_prestat_dir_name %i", wasmFd);
    GET_EXECUTING_MODULE_AND_CHECK(execEnv);
    module->validateNativePointer(path, pathLen);

    int returnValue;
    sgx_status_t sgxReturnValue;
    std::string retPath;
    retPath.resize(pathLen);
    if ((sgxReturnValue = ocallWasiFdPrestatDirName(
           &returnValue, wasmFd, (char*)retPath.c_str(), pathLen)) !=
        SGX_SUCCESS) {
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

// To implement fd_read we need to:
// 1. Validate the wasm pointers
// 2. Translate the wasm iovecs into native iovecs
// 3. Do the actual reading from the fd
// 4. Return the bytes read by populating the return pointer
// We do steps 1 and 2 inside the enclave, and we do an OCall for step 3
static int32_t wasi_fd_read(wasm_exec_env_t execEnv,
                            int32_t wasmFd,
                            iovec_app_t* ioVecBuffWasm,
                            int32_t ioVecCountWasm,
                            int32_t* bytesRead)
{
    SPDLOG_DEBUG_SGX("S - wasi_fd_read %i", wasmFd);
    GET_EXECUTING_MODULE_AND_CHECK(execEnv);

    // Check pointers
    module->validateNativePointer(reinterpret_cast<void*>(ioVecBuffWasm),
                                  sizeof(iovec_app_t) * ioVecCountWasm);
    module->validateNativePointer(bytesRead, sizeof(int32_t));

    // Translate the wasm iovecs into native iovecs and serialise to transfer
    // as an OCall. For a detailed explanation of the serialisation, read the
    // comment in wasi_fd_write
    size_t totalBasesSize = 0;
    for (int i = 0; i < ioVecCountWasm; i++) {
        totalBasesSize += ioVecBuffWasm[i].buffLen;
    }

    std::vector<uint8_t> ioVecBases(totalBasesSize);
    int32_t offset = 0;
    std::vector<int32_t> ioVecOffsets(ioVecCountWasm);
    for (int i = 0; i < ioVecCountWasm; i++) {
        ioVecOffsets[i] = offset;
        offset += ioVecBuffWasm[i].buffLen;
    }

    int returnValue;
    sgx_status_t sgxReturnValue;
    if ((sgxReturnValue = ocallWasiFdRead(&returnValue,
                                          wasmFd,
                                          ioVecBases.data(),
                                          totalBasesSize,
                                          ioVecOffsets.data(),
                                          ioVecCountWasm,
                                          bytesRead)) != SGX_SUCCESS) {
        SET_ERROR(FAASM_SGX_OCALL_ERROR(sgxReturnValue));
    }

    // Copy the return values into the wasm iovec buffer
    offset = 0;
    for (int i = 0; i < ioVecCountWasm; i++) {
        module->validateWasmOffset(ioVecBuffWasm[i].buffOffset,
                                   sizeof(char) * ioVecBuffWasm[i].buffLen);
        memcpy(module->wamrWasmPointerToNative(ioVecBuffWasm[i].buffOffset),
               ioVecBases.data() + offset,
               ioVecBuffWasm[i].buffLen);
        offset += ioVecBuffWasm[i].buffLen;
    }

    return returnValue;
}

static int32_t wasi_fd_readdir(wasm_exec_env_t execEnv,
                               int32_t wasmFd,
                               char* buf,
                               uint32_t bufLen,
                               int64_t startCookie,
                               int32_t* resSizePtr)
{
    SPDLOG_DEBUG_SGX("S - wasi_fd_readdir %i", wasmFd);
    GET_EXECUTING_MODULE_AND_CHECK(execEnv);

    module->validateNativePointer(reinterpret_cast<void*>(resSizePtr),
                                  sizeof(int32_t));

    int returnValue;
    sgx_status_t sgxReturnValue;
    if ((sgxReturnValue = ocallWasiFdReadDir(
           &returnValue, wasmFd, buf, bufLen, startCookie, resSizePtr)) !=
        SGX_SUCCESS) {
        SET_ERROR(FAASM_SGX_OCALL_ERROR(sgxReturnValue));
    }

    return returnValue;
}

static int wasi_fd_seek(wasm_exec_env_t execEnv,
                        int32_t wasmFd,
                        int64_t offset,
                        int32_t whence,
                        __wasi_filesize_t* newOffset)
{
    SPDLOG_DEBUG_SGX("S - wasi_fd_seek %i", wasmFd);
    GET_EXECUTING_MODULE_AND_CHECK(execEnv);

    module->validateNativePointer(newOffset, sizeof(__wasi_filesize_t));

    int returnValue;
    sgx_status_t sgxReturnValue;
    if ((sgxReturnValue = ocallWasiFdSeek(
           &returnValue, wasmFd, offset, whence, newOffset)) != SGX_SUCCESS) {
    }

    return returnValue;
}

// For fd_fdstat_get we need to:
// 1. Validate the return pointer provided by the address
// 2. Retrieve the stat for the corresponding file descriptor
// 3. Copy the results into the provided pointer
// We do steps 1 and 3 inside the enclave, and do an OCall for step 2
static int32_t wasi_fd_fdstat_get(wasm_exec_env_t execEnv,
                                  int32_t wasmFd,
                                  __wasi_fdstat_t* statWasm)
{
    SPDLOG_DEBUG_SGX("S - wasi_fd_fdstat_get %i", wasmFd);

    GET_EXECUTING_MODULE_AND_CHECK(execEnv);

    module->validateNativePointer(statWasm, sizeof(__wasi_fdstat_t));

    uint8_t wasiFileType;
    uint64_t rightsBase;
    uint64_t rightsInheriting;
    int32_t returnValue;
    sgx_status_t sgxReturnValue;
    if ((sgxReturnValue = ocallWasiFdFdstatGet(&returnValue,
                                               wasmFd,
                                               &wasiFileType,
                                               &rightsBase,
                                               &rightsInheriting)) !=
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
    SPDLOG_DEBUG_SGX("S - wasi_fd_fdstat_set_flags");

    UNIMPLEMENTED_WASM_INTRINSIC("wasi_fd_fdstat_set_flags");
}

static int32_t wasi_fd_fdstat_set_rights(wasm_exec_env_t exec_env,
                                         int32_t a,
                                         int64_t b,
                                         int64_t c)
{
    UNIMPLEMENTED_WASM_INTRINSIC("fd_fdstat_set_rights");
}

// To implement fd_write we need to:
// 1. Validate the wasm pointers
// 2. Translate the wasm iovecs into native iovecs
// 3. Do the actual writing to the fd
// 4. Return the bytes written by populating the return pointer
// We do steps 1 and 2 inside the enclave, and we do an OCall for step 3
static int wasi_fd_write(wasm_exec_env_t execEnv,
                         int32_t wasmFd,
                         iovec_app_t* ioVecBuffWasm,
                         int32_t ioVecCountWasm,
                         int32_t* bytesWritten)
{
    SPDLOG_DEBUG_SGX("S - wasi_fd_write %i", wasmFd);
    GET_EXECUTING_MODULE_AND_CHECK(execEnv);

    // Check pointers
    module->validateNativePointer(reinterpret_cast<void*>(ioVecBuffWasm),
                                  sizeof(iovec_app_t) * ioVecCountWasm);
    module->validateNativePointer(bytesWritten, sizeof(int32_t));

    // Translate the wasm iovecs into native iovecs and serialise to transfer
    // as an OCall. Here we use that an iovec_app_t is a struct with a
    // uint8_t pointer and a size_t length. To serialise it, we copy the
    // contents of each base into a flattened array, and record the offset
    // each one starts at. Using these offsets we can reconstruct the length
    size_t totalBasesSize = 0;
    for (int i = 0; i < ioVecCountWasm; i++) {
        totalBasesSize += ioVecBuffWasm[i].buffLen;
    }

    std::vector<uint8_t> ioVecBases(totalBasesSize);
    int32_t offset = 0;
    std::vector<int32_t> ioVecOffsets(ioVecCountWasm);
    for (int i = 0; i < ioVecCountWasm; i++) {
        module->validateWasmOffset(ioVecBuffWasm[i].buffOffset,
                                   sizeof(char) * ioVecBuffWasm[i].buffLen);

        memcpy(ioVecBases.data() + offset,
               module->wamrWasmPointerToNative(ioVecBuffWasm[i].buffOffset),
               ioVecBuffWasm[i].buffLen);
        ioVecOffsets.at(i) = offset;
        offset += ioVecBuffWasm[i].buffLen;
    }

    int returnValue;
    sgx_status_t sgxReturnValue;
    if ((sgxReturnValue = ocallWasiFdWrite(&returnValue,
                                           wasmFd,
                                           ioVecBases.data(),
                                           totalBasesSize,
                                           ioVecOffsets.data(),
                                           ioVecCountWasm,
                                           bytesWritten)) != SGX_SUCCESS) {
    }

    return returnValue;
}

// To run path_filestat_get we need to:
// 1. Get the stat from the path
// 2. Copy the contents into WASM memory
// We do step 1 in an OCall and step 2 inside the enclave
static int32_t wasi_path_filestat_get(wasm_exec_env_t execEnv,
                                      int32_t wasmFd,
                                      int32_t lookupFlags,
                                      char* path,
                                      int32_t pathLen,
                                      __wasi_filestat_t* statWasm)
{
    GET_EXECUTING_MODULE_AND_CHECK(execEnv);
    module->validateNativePointer(path, pathLen);
    SPDLOG_DEBUG_SGX("S - wasi_path_filestat_get %i (%s)", wasmFd, path);

    int32_t returnValue;
    sgx_status_t sgxReturnValue;
    if ((sgxReturnValue = ocallWasiPathFilestatGet(&returnValue,
                                                   wasmFd,
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
                                  int32_t wasmFd,
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
    SPDLOG_DEBUG_SGX("S - wasi_path_readlink %i (%s)", wasmFd, path);

    int32_t returnValue;
    sgx_status_t sgxReturnValue;
    if ((sgxReturnValue = ocallWasiPathReadlink(
           &returnValue, wasmFd, path, pathLen, buf, bufLen, resBytesUsed)) !=
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
    SPDLOG_DEBUG_SGX("S - wasi_path_remove_directory");

    UNIMPLEMENTED_WASM_INTRINSIC("wasi_path_remove_directory");
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

    UNIMPLEMENTED_WASM_INTRINSIC("wasi_path_symlink");
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
    REG_WASI_NATIVE_FUNC(fd_close, "(i)i"),
    REG_WASI_NATIVE_FUNC(fd_fdstat_get, "(ii)i"),
    REG_WASI_NATIVE_FUNC(fd_fdstat_set_flags, "(ii)i"),
    REG_WASI_NATIVE_FUNC(fd_fdstat_set_rights, "(iII)i"),
    REG_WASI_NATIVE_FUNC(fd_filestat_get, "(i*)i"),
    REG_WASI_NATIVE_FUNC(fd_prestat_dir_name, "(i*~)i"),
    REG_WASI_NATIVE_FUNC(fd_prestat_get, "(i*)i"),
    REG_WASI_NATIVE_FUNC(fd_read, "(i*i*)i"),
    REG_WASI_NATIVE_FUNC(fd_readdir, "(i*~I*)i"),
    REG_WASI_NATIVE_FUNC(fd_seek, "(iIi*)i"),
    REG_WASI_NATIVE_FUNC(fd_write, "(iiii)i"),
    REG_WASI_NATIVE_FUNC(path_filestat_get, "(ii*~*)i"),
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
