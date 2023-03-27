#include <faabric/proto/faabric.pb.h>
#include <faabric/util/logging.h>

#include <wamr/WAMRWasmModule.h>
#include <wamr/native.h>
#include <wasm/WasmExecutionContext.h>
#include <wasm/WasmModule.h>
#include <wasm_export.h>

namespace wasm {
static int32_t __sbrk_wrapper(wasm_exec_env_t exec_env, int32_t increment)
{
    // Note trace logging here as this is called a lot
    SPDLOG_TRACE("S - __sbrk - {}", increment);
    WasmModule* module = getExecutingModule();
    uint32_t oldBrk = module->getCurrentBrk();

    if (increment == 0) {
        return oldBrk;
    }

    if (increment < 0) {
        module->shrinkMemory(-1 * increment);
        return oldBrk;
    }

    return module->growMemory(increment);
}

static int32_t mmap_wrapper(wasm_exec_env_t exec_env,
                            int32_t addr,
                            int32_t length,
                            int32_t prot,
                            int32_t flags,
                            int32_t fd,
                            int64_t offset)
{
    SPDLOG_TRACE(
      "S - mmap - {} {} {} {} {} {}", addr, length, prot, flags, fd, offset);

    if (offset != 0) {
        SPDLOG_WARN("WARNING: ignoring non-zero mmap offset ({})", offset);
    }

    // Likewise with the address hint
    if (addr != 0) {
        SPDLOG_WARN("WARNING: ignoring mmap hint at {}", addr);
    }

    WAMRWasmModule* module = getExecutingWAMRModule();

    if (fd != -1) {
        // If fd is provided, we're mapping a file into memory
        storage::FileDescriptor& fileDesc =
          module->getFileSystem().getFileDescriptor(fd);
        return module->mmapFile(fileDesc.getLinuxFd(), length);
    }

    // If fd not provided, map memory directly
    return module->mmapMemory(length);
}

static int32_t munmap_wrapper(wasm_exec_env_t exec_env,
                              int32_t addr,
                              int32_t length)
{
    SPDLOG_DEBUG("S - munmap - {} {}", addr, length);

    WAMRWasmModule* executingModule = getExecutingWAMRModule();
    executingModule->unmapMemory(addr, length);

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
