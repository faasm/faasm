#include <enclave/inside/EnclaveWasmModule.h>
#include <enclave/inside/native.h>
#include <enclave/inside/ocalls.h>

#include <string>

namespace sgx {
static int32_t getrlimit_wrapper(wasm_exec_env_t exec_env, int32_t a, int32_t b)
{
    SPDLOG_DEBUG_SGX("S - getrlimit");
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

// ----- WASI Symbols -----

static int wasi_args_get(wasm_exec_env_t execEnv,
                         uint32_t* argvOffsetsWasm,
                         char* argvBuffWasm)
{
    SPDLOG_DEBUG_SGX("S - wasi_args_get");

    GET_EXECUTING_MODULE_AND_CHECK(execEnv);

    module->writeArgvToWamrMemory(argvOffsetsWasm, argvBuffWasm);

    return 0;
}

static int wasi_args_sizes_get(wasm_exec_env_t execEnv,
                               uint32_t* argcWasm,
                               uint32_t* argvBuffSizeWasm)
{
    SPDLOG_DEBUG_SGX("S - wasi_args_sizes_get");

    GET_EXECUTING_MODULE_AND_CHECK(execEnv);

    *argcWasm = module->getArgc();
    *argvBuffSizeWasm = module->getArgvBufferSize();

    return 0;
}

static void wasi_proc_exit(wasm_exec_env_t execEnv, int returnCode)
{
    SPDLOG_DEBUG_SGX("S - proc_exit %i", returnCode);

    std::shared_ptr<wasm::EnclaveWasmModule> module = wasm::getExecutingEnclaveWasmModule(execEnv);
    if (module == nullptr) {
        SPDLOG_ERROR_SGX("Can't find executing module");
        returnCode = 1;
    }
    std::string resStr = WAMR_EXIT_PREFIX;
    resStr += std::to_string(returnCode);
    wasm_runtime_set_exception(module->getModuleInstance(), resStr.c_str());
}

static NativeSymbol wasiNs[] = {
    REG_WASI_NATIVE_FUNC(args_get, "(**)i"),
    REG_WASI_NATIVE_FUNC(args_sizes_get, "(**)i"),
    REG_WASI_NATIVE_FUNC(proc_exit, "(i)"),
};

uint32_t getFaasmWasiEnvApi(NativeSymbol** nativeSymbols)
{
    *nativeSymbols = wasiNs;
    return sizeof(wasiNs) / sizeof(NativeSymbol);
}
}
