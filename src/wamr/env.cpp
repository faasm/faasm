#include <faabric/util/logging.h>
#include <storage/FileDescriptor.h>
#include <wamr/WAMRWasmModule.h>
#include <wamr/native.h>

#include <stdexcept>

#include <wasm_export.h>
#include <wasmtime_ssp.h>

namespace wasm {
uint32_t wasi_args_get(wasm_exec_env_t exec_env,
                       uint32_t* argvOffsetsWasm,
                       char* argvBuffWasm)
{
    SPDLOG_DEBUG("S - args_get");

    WAMRWasmModule* module = getExecutingWAMRModule();
    module->writeArgvToWamrMemory(argvOffsetsWasm, argvBuffWasm);

    return __WASI_ESUCCESS;
}

uint32_t wasi_args_sizes_get(wasm_exec_env_t exec_env,
                             uint32_t* argcWasm,
                             uint32_t* argvBuffSizeWasm)
{
    SPDLOG_DEBUG("S - args_sizes_get");

    WAMRWasmModule* module = getExecutingWAMRModule();

    module->validateNativePointer(argcWasm, sizeof(uint32_t));
    module->validateNativePointer(argvBuffSizeWasm, sizeof(uint32_t));

    *argcWasm = module->getArgc();
    *argvBuffSizeWasm = module->getArgvBufferSize();

    return __WASI_ESUCCESS;
}

uint32_t wasi_environ_get(wasm_exec_env_t exec_env,
                          uint32_t* envOffsetsWasm,
                          char* envBuffWasm)
{
    SPDLOG_DEBUG("S - environ_get");

    WAMRWasmModule* module = getExecutingWAMRModule();
    module->writeWasmEnvToWamrMemory(envOffsetsWasm, envBuffWasm);

    return __WASI_ESUCCESS;
}

uint32_t wasi_environ_sizes_get(wasm_exec_env_t exec_env,
                                int32_t* envCountWasm,
                                int32_t* envBufferSizeWasm)
{
    SPDLOG_DEBUG("S - environ_sizes_get");

    WAMRWasmModule* module = getExecutingWAMRModule();
    WasmEnvironment& wasmEnv = module->getWasmEnvironment();

    module->validateNativePointer(envCountWasm, sizeof(uint32_t));
    module->validateNativePointer(envBufferSizeWasm, sizeof(uint32_t));

    *envCountWasm = wasmEnv.getEnvCount();
    *envBufferSizeWasm = wasmEnv.getEnvBufferSize();

    return __WASI_ESUCCESS;
}

void wasi_proc_exit(wasm_exec_env_t exec_env, int32_t retCode)
{
    SPDLOG_DEBUG("S - proc_exit {}", retCode);

    WAMRWasmModule* module = getExecutingWAMRModule();
    std::string resStr = WAMR_EXIT_PREFIX;
    resStr += std::to_string(retCode);
    wasm_runtime_set_exception(module->getModuleInstance(), resStr.c_str());
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
