#include <faabric/util/logging.h>
#include <storage/FileDescriptor.h>
#include <wamr/WAMRWasmModule.h>
#include <wamr/native.h>

#include <stdexcept>

// WAMR includes
#include <wasm_export.h>
#include <wasmtime_ssp.h>

namespace wasm {
uint32_t wasi_args_get(wasm_exec_env_t exec_env,
                       int32_t* argvOffsetsApp,
                       char* argvBuffApp)
{
    SPDLOG_DEBUG("S - args_get");

    WAMRWasmModule* module = getExecutingWAMRModule();
    const std::vector<std::string>& argvsHost = module->getArgv();

    VALIDATE_NATIVE_ADDR(argvOffsetsApp, argvsHost.size() * sizeof(int32_t));

    for (const auto& argvHost : argvsHost) {
        // To validate the buffer we must account for the null terminator
        VALIDATE_NATIVE_ADDR(argvBuffApp, argvHost.size() + 1);

        // Copy contents to buffer and update offsets
        std::copy(argvHost.begin(), argvHost.end(), argvBuffApp);
        *argvOffsetsApp = ADDR_NATIVE_TO_APP(argvBuffApp);

        argvBuffApp += argvHost.size() + 1;
        argvOffsetsApp += sizeof(int32_t);
    }

    return __WASI_ESUCCESS;
}

uint32_t wasi_args_sizes_get(wasm_exec_env_t exec_env,
                             uint32_t* argcApp,
                             uint32_t* argcBuffSizeApp)
{
    SPDLOG_DEBUG("S - args_sizes_get");

    WAMRWasmModule* module = getExecutingWAMRModule();

    VALIDATE_NATIVE_ADDR(argcApp, sizeof(uint32_t));
    VALIDATE_NATIVE_ADDR(argcBuffSizeApp, sizeof(uint32_t));

    *argcApp = module->getArgc();
    *argcBuffSizeApp = module->getArgvBufferSize();

    return __WASI_ESUCCESS;
}

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

uint32_t wasi_proc_exit(wasm_exec_env_t exec_env, int32_t retCode)
{
    SPDLOG_DEBUG("S - proc_exit {}", retCode);
    throw(WasmExitException(retCode));
}

static NativeSymbol wasiNs[] = {
    REG_WASI_NATIVE_FUNC(args_get, "(**)i"),
    REG_WASI_NATIVE_FUNC(args_sizes_get, "(**)i"),
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
