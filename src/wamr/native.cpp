#include <wamr/native.h>
#include <wasm_export.h>
#include <wasm_native.h>

namespace wasm {
void doSymbolRegistration(uint32_t (*f)(NativeSymbol** ns))
{
    NativeSymbol* symbols;
    uint32_t nSymbols = f(&symbols);
    wasm_native_register_natives("env", symbols, nSymbols);
}

void doWasiSymbolRegistration(uint32_t (*f)(NativeSymbol** ns))
{
    NativeSymbol* symbols;
    uint32_t nSymbols = f(&symbols);
    wasm_native_register_natives("wasi_snapshot_preview1", symbols, nSymbols);
}

void initialiseWAMRNatives()
{
    // Register native symbols
    doSymbolRegistration(getFaasmChainingApi);
    doSymbolRegistration(getFaasmDynlinkApi);
    doSymbolRegistration(getFaasmFilesystemApi);
    doSymbolRegistration(getFaasmFunctionsApi);
    doSymbolRegistration(getFaasmMemoryApi);
    doSymbolRegistration(getFaasmPthreadApi);
    doSymbolRegistration(getFaasmStateApi);
    doSymbolRegistration(getFaasmStubs);

    // Register wasi symbols
    doWasiSymbolRegistration(getFaasmWasiEnvApi);
    doWasiSymbolRegistration(getFaasmWasiFilesystemApi);
    doWasiSymbolRegistration(getFaasmWasiTimingApi);
}
}
