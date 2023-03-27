#include <faabric/util/logging.h>
#include <storage/FileDescriptor.h>
#include <wamr/WAMRWasmModule.h>
#include <wamr/native.h>

#include <stdexcept>
#include <sys/random.h>

#include <wasm_export.h>
#include <wasmtime_ssp.h>

namespace wasm {
static int32_t getrlimit_wrapper(wasm_exec_env_t exec_env, int32_t a, int32_t b)
{
    SPDLOG_DEBUG("S - getrlimit");
    // We ignore calls to getrlimit, this may break some functionalities
    return 0;
}

static NativeSymbol ns[] = {
    REG_NATIVE_FUNC(getrlimit, "(ii)i"),
};

uint32_t getFaasmEnvApi(NativeSymbol** nativeSymbols)
{
    *nativeSymbols = ns;
    return sizeof(ns) / sizeof(NativeSymbol);
}

// ---------- WASI symbols ----------
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

void wasi_proc_exit(wasm_exec_env_t execEnv, int32_t retCode)
{
    SPDLOG_DEBUG("S - proc_exit {}", retCode);
    // WAMRWasmModule* module = getExecutingWAMRModule();
    WASMModuleInstanceCommon* module_inst = wasm_runtime_get_module_inst(execEnv);
    WASIContext* wasiCtx = wasm_runtime_get_wasi_ctx(module_inst);
    wasm_runtime_set_exception(module_inst, "wasi proc exit");
    wasiCtx->exit_code = retCode;
}

static uint32_t wasi_random_get(wasm_exec_env_t exec_env,
                                void* buf,
                                uint32_t bufLen)
{
    SPDLOG_DEBUG("S - random_get");

    getrandom(buf, bufLen, 0);

    return __WASI_ESUCCESS;
}

static NativeSymbol wasiNs[] = {
    REG_WASI_NATIVE_FUNC(args_get, "(**)i"),
    REG_WASI_NATIVE_FUNC(args_sizes_get, "(**)i"),
    REG_WASI_NATIVE_FUNC(environ_get, "(**)i"),
    REG_WASI_NATIVE_FUNC(environ_sizes_get, "(**)i"),
    REG_WASI_NATIVE_FUNC(proc_exit, "(i)"),
    REG_WASI_NATIVE_FUNC(random_get, "(*~)i"),
};

uint32_t getFaasmWasiEnvApi(NativeSymbol** nativeSymbols)
{
    *nativeSymbols = wasiNs;
    return sizeof(wasiNs) / sizeof(NativeSymbol);
}
}
