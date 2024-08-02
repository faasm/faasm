#include <enclave/inside/EnclaveWasmModule.h>
#include <enclave/inside/native.h>
#include <wamr/types.h>

namespace sgx {
static int wasi_fd_close(wasm_exec_env_t execEnv, int a)
{
    UNIMPLEMENTED_WASM_INTRINSIC("fd_close");
}

static int wasi_fd_seek(wasm_exec_env_t execEnv, int a, int64 b, int c, int d)
{
    UNIMPLEMENTED_WASM_INTRINSIC("fd_seek");
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
    SPDLOG_DEBUG_SGX("SSSSSSSSSSSSSSS - wasi_fd_fdstat_get %i", wasmFd);

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

    // uint8_t ioVecBases[totalBasesSize];
    std::vector<uint8_t> ioVecBases(totalBasesSize);
    int32_t offset = 0;
    std::vector<int32_t> ioVecOffsets(ioVecCountWasm);
    // int32_t ioVecOffsets[ioVecCountWasm];
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

static NativeSymbol wasiNs[] = {
    REG_WASI_NATIVE_FUNC(fd_close, "(i)i"),
    REG_WASI_NATIVE_FUNC(fd_fdstat_get, "(ii)i"),
    REG_WASI_NATIVE_FUNC(fd_fdstat_set_rights, "(iII)i"),
    REG_WASI_NATIVE_FUNC(fd_seek, "(iIii)i"),
    REG_WASI_NATIVE_FUNC(fd_write, "(iiii)i"),
};

uint32_t getFaasmWasiFilesystemApi(NativeSymbol** nativeSymbols)
{
    *nativeSymbols = wasiNs;
    return sizeof(wasiNs) / sizeof(NativeSymbol);
}
}
