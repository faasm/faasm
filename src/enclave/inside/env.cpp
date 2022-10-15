#include <enclave/inside/EnclaveWasmModule.h>
#include <enclave/inside/native.h>
#include <enclave/inside/ocalls.h>

#include <sgx_trts.h>
#include <sstream>
#include <string>

#define MAX_ENV_LENGTH 2048

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

    return __WASI_ESUCCESS;
}

static int wasi_args_sizes_get(wasm_exec_env_t execEnv,
                               uint32_t* argcWasm,
                               uint32_t* argvBuffSizeWasm)
{
    SPDLOG_DEBUG_SGX("S - wasi_args_sizes_get");

    GET_EXECUTING_MODULE_AND_CHECK(execEnv);

    *argcWasm = module->getArgc();
    *argvBuffSizeWasm = module->getArgvBufferSize();

    return __WASI_ESUCCESS;
}

// To implement environ_get we:
// 1. Make an OCall to get the environment variables serialised
// 2. Deserialise the env. variables to a vector to use the mixin method
int32_t wasi_environ_get(wasm_exec_env_t execEnv,
                         uint32_t* envOffsetsWasm,
                         char* envBuffWasm)
{
    SPDLOG_DEBUG_SGX("S - wasi_environ_get");
    GET_EXECUTING_MODULE_AND_CHECK(execEnv);

    // We serialise the environment variables to a char buffer, where each
    // string is separated by a null terminator
    char envBuf[MAX_ENV_LENGTH];
    int32_t bytesWritten;
    int32_t returnValue;
    sgx_status_t sgxReturnValue;
    if ((sgxReturnValue = ocallWasiEnvironGet(
           &returnValue, envBuf, MAX_ENV_LENGTH, &bytesWritten)) !=
        SGX_SUCCESS) {
        SET_ERROR(FAASM_SGX_OCALL_ERROR(sgxReturnValue));
    }

    // We consume the buffer until we have written all the bytes
    std::vector<std::string> wasmEnv;
    int32_t numWritten = 0;
    while (numWritten < bytesWritten) {
        wasmEnv.push_back(std::string(envBuf + numWritten));
        numWritten += wasmEnv.back().size() + 1;
    }
    module->writeStringArrayToMemory(wasmEnv, envOffsetsWasm, envBuffWasm);

    return __WASI_ESUCCESS;
}

uint32_t wasi_environ_sizes_get(wasm_exec_env_t execEnv,
                                int32_t* envCountWasm,
                                int32_t* envBufferSizeWasm)
{
    SPDLOG_DEBUG_SGX("S - wasi_environ_sizes_get");
    GET_EXECUTING_MODULE_AND_CHECK(execEnv);
    module->validateNativePointer(envCountWasm, sizeof(uint32_t));
    module->validateNativePointer(envBufferSizeWasm, sizeof(uint32_t));

    int32_t returnValue;
    sgx_status_t sgxReturnValue;
    if ((sgxReturnValue = ocallWasiEnvironSizesGet(
           &returnValue, envCountWasm, envBufferSizeWasm)) != SGX_SUCCESS) {
        SET_ERROR(FAASM_SGX_OCALL_ERROR(sgxReturnValue));
    }

    return returnValue;
}

static void wasi_proc_exit(wasm_exec_env_t execEnv, int returnCode)
{
    SPDLOG_DEBUG_SGX("S - proc_exit %i", returnCode);

    std::shared_ptr<wasm::EnclaveWasmModule> module =
      wasm::getExecutingEnclaveWasmModule(execEnv);
    if (module == nullptr) {
        SPDLOG_ERROR_SGX("Can't find executing module");
        returnCode = 1;
    }
    std::string resStr = WAMR_EXIT_PREFIX;
    resStr += std::to_string(returnCode);
    wasm_runtime_set_exception(module->getModuleInstance(), resStr.c_str());
}

static uint32_t wasi_random_get(wasm_exec_env_t execEnv,
                                void* buf,
                                uint32_t bufLen)
{
    SPDLOG_DEBUG_SGX("S - wasi_random_get");
    GET_EXECUTING_MODULE_AND_CHECK(execEnv);
    module->validateNativePointer(buf, bufLen);

    sgx_read_rand(reinterpret_cast<unsigned char*>(buf), bufLen);

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
