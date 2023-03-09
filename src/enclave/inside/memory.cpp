#include <enclave/inside/EnclaveWasmModule.h>
#include <enclave/inside/native.h>

namespace sgx {
static int32_t __sbrk_wrapper(wasm_exec_env_t execEnv, int32_t increment)
{
    SPDLOG_DEBUG_SGX("S - __sbrk - %i", increment);
    GET_EXECUTING_MODULE_AND_CHECK(execEnv);
    uint32_t oldBrk = module->getCurrentBrk();

    // TODO(csegarragonz): ideally, we'd share shrinkMemory and growMemory
    // across all runtimes
    if (increment == 0) {
        return oldBrk;
    }
    if (increment < 0) {
        module->shrinkMemory(-1 * increment);
        return oldBrk;
    }
    return module->growMemory(increment);
}

static int32_t mmap_wrapper(wasm_exec_env_t execEnv,
                            int32_t addr,
                            int32_t length,
                            int32_t prot,
                            int32_t flags,
                            int32_t fd,
                            int64_t offset)
{
    SPDLOG_DEBUG_SGX(
      "S - mmap - %i %i %i %i %i %li", addr, length, prot, flags, fd, offset);

    if (offset != 0) {
        SPDLOG_DEBUG_SGX("WARNING: ignoring non-zero mmap offset (%li)",
                         offset);
    }

    // Likewise with the address hint
    if (addr != 0) {
        SPDLOG_DEBUG_SGX("WARNING: ignoring mmap hint at %i", addr);
    }

    GET_EXECUTING_MODULE_AND_CHECK(execEnv);

    if (fd != -1) {
        SPDLOG_ERROR_SGX("ERROR: mmaping file stil not supported in SGX");
        return -1;
    }

    // If fd not provided, map memory directly
    return module->mmapMemory(length);
}

static int32_t munmap_wrapper(wasm_exec_env_t execEnv,
                              int32_t addr,
                              int32_t length)
{
    SPDLOG_DEBUG_SGX("S - munmap - %i %i", addr, length);

    GET_EXECUTING_MODULE_AND_CHECK(execEnv);
    module->unmapMemory(addr, length);

    return 0;
}

static NativeSymbol ns[] = {
    REG_NATIVE_FUNC(__sbrk, "(i)i"),
    REG_NATIVE_FUNC(mmap, "(iiiiiI)i"),
    REG_NATIVE_FUNC(munmap, "(ii)i"),
};

uint32_t getFaasmMemoryApi(NativeSymbol** nativeSymbols)
{
    *nativeSymbols = ns;
    return sizeof(ns) / sizeof(NativeSymbol);
}
}
