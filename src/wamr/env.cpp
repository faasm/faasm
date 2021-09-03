#include <faabric/util/logging.h>
#include <storage/FileDescriptor.h>
#include <wamr/WAMRWasmModule.h>
#include <wamr/native.h>

#include <stdexcept>

// WAMR includes
#include <wasm_export.h>
#include <wasmtime_ssp.h>

namespace wasm {
uint32_t wasi_environ_get(wasm_exec_env_t exec_env,
                          int32_t* envOffsetsApp,
                          char* envBuffApp)
{
    SPDLOG_DEBUG("S - environ_get");

    WAMRWasmModule* module = getExecutingWAMRModule();
    WasmEnvironment wasmEnv = module->getWasmEnvironment();
    const std::vector<std::string>& envVars = wasmEnv.getVars();

    // Validate offset vector at once
    VALIDATE_NATIVE_ADDR(envOffsetsApp, envVars.size() * sizeof(int32_t));

    for (const auto& envVar : envVars) {
        // To validate the buffer we must account for the null terminator
        VALIDATE_NATIVE_ADDR(envBuffApp, envVar.size() + 1)

        // Copy contents to buffer and update offsets
        std::copy(envVar.begin(), envVar.end(), envBuffApp);
        *envOffsetsApp = ADDR_NATIVE_TO_APP(envBuffApp);

        envBuffApp += envVar.size() + 1;
        envOffsetsApp += sizeof(int32_t);
    }

    return __WASI_ESUCCESS;
}

uint32_t wasi_environ_sizes_get(wasm_exec_env_t exec_env,
                                int32_t* envCountApp,
                                int32_t* envBufferSizeApp)
{
    SPDLOG_DEBUG("S - environ_sizes_get");

    WAMRWasmModule* module = getExecutingWAMRModule();
    WasmEnvironment& wasmEnv = module->getWasmEnvironment();

    VALIDATE_NATIVE_ADDR(envCountApp, sizeof(uint32_t))
    VALIDATE_NATIVE_ADDR(envBufferSizeApp, sizeof(uint32_t))

    *envCountApp = wasmEnv.getEnvCount();
    *envBufferSizeApp = wasmEnv.getEnvBufferSize();

    return __WASI_ESUCCESS;
}

uint32_t wasi_proc_exit(wasm_exec_env_t exec_env)
{
    SPDLOG_DEBUG("S - proc_exit");
    throw std::runtime_error("proc_exit not implemented");
    return 0;
}

static NativeSymbol wasiNs[] = {
    REG_WASI_NATIVE_FUNC(environ_get, "(**)i"),
    REG_WASI_NATIVE_FUNC(environ_sizes_get, "(**)i"),
    REG_WASI_NATIVE_FUNC(proc_exit, "(i)"),
};

uint32_t getFaasmWasiEnvApi(NativeSymbol** nativeSymbols)
{
    *nativeSymbols = wasiNs;
    return sizeof(wasiNs) / sizeof(NativeSymbol);
}
}
