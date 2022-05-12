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
    } else if (increment < 0) {
        module->shrinkMemory(-1 * increment);
        return oldBrk;
    } else {
        return module->growMemory(increment);
    }
}

static int32_t mmap_wrapper(wasm_exec_env_t exec_env,
                            void* addr,
                            int32_t length,
                            int32_t prot,
                            int32_t flags,
                            int32_t fd,
                            int32_t offset)
{
    SPDLOG_TRACE(
      "S - mmap - {} {} {} {} {} {}", addr, length, prot, flags, fd, offset);
    WasmModule* module = getExecutingModule();
    return module->mmapMemory(length);
}

static NativeSymbol ns[] = {
    REG_NATIVE_FUNC(__sbrk, "(i)i"),
    REG_NATIVE_FUNC(mmap, "(*iiiii)i"),
};

uint32_t getFaasmMemoryApi(NativeSymbol** nativeSymbols)
{
    *nativeSymbols = ns;
    return sizeof(ns) / sizeof(NativeSymbol);
}
}
