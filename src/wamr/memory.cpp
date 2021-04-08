#include <faabric/proto/faabric.pb.h>
#include <wamr/WAMRWasmModule.h>
#include <wamr/native.h>
#include <wasm/WasmModule.h>
#include <wasm_export.h>

namespace wasm {
/**
 * Pushes the state for the given key
 */
static int32_t __sbrk_wrapper(wasm_exec_env_t exec_env, int32_t increment)
{
    faabric::util::getLogger()->debug("S - __sbrk - {}", increment);
    WAMRWasmModule* module = getExecutingWAMRModule();
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

static NativeSymbol ns[] = {
    REG_NATIVE_FUNC(__sbrk, "(i)i"),
};

uint32_t getFaasmMemoryApi(NativeSymbol** nativeSymbols)
{
    *nativeSymbols = ns;
    return sizeof(ns) / sizeof(NativeSymbol);
}
}
