#pragma once

#include <wamr/WAMRModuleMixin.h>
#include <wasm/WasmCommon.h>

#include <iwasm/aot/aot_runtime.h>
#include <wasm_runtime_common.h>

#include <atomic>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>

namespace wasm {

/*
 * Abstraction around a WebAssembly module running inside an SGX enclave with
 * the WAMR runtime.  */
class EnclaveWasmModule : public WAMRModuleMixin<EnclaveWasmModule>
{
  public:
    static bool initialiseWAMRGlobally();

    EnclaveWasmModule();

    ~EnclaveWasmModule();

    bool loadWasm(void* wasmOpCodePtr, uint32_t wasmOpCodeSize);

    uint32_t callFunction(uint32_t argcIn, char** argvIn);

    // TODO: remove duplication with WAMRWasmModule
    int executeWasmFunction(const std::string& funcName);

    WASMModuleInstanceCommon* getModuleInstance();

    // ---- argc/arv ----

    uint32_t getArgc() const;

    std::vector<std::string> getArgv();

    size_t getArgvBufferSize() const;

    // ---- Memory management ----
    // TODO(csegarragonz): what bits of the memory management can we
    // de-duplicate using the WAMR's module mixin?

    uint32_t getCurrentBrk() const;

    size_t getMemorySizeBytes();

    uint8_t* getMemoryBase();

    size_t getMaxMemoryPages();

    uint32_t growMemory(size_t nBytes);

    uint32_t shrinkMemory(size_t nBytes);

  private:
    char errorBuffer[WAMR_ERROR_BUFFER_SIZE];

    WASMModuleCommon* wasmModule;
    WASMModuleInstanceCommon* moduleInstance;

    // Argc/argv
    uint32_t argc;
    std::vector<std::string> argv;
    size_t argvBufferSize;

    void prepareArgcArgv(uint32_t argcIn, char** argvIn);

    // Memory management
    std::atomic<uint32_t> currentBrk = 0;
    std::mutex moduleMutex;
};

// Data structure to keep track of the modules currently loaded in the enclave.
// And mutex to control concurrent accesses. Both objects have external
// definition as they have to be accessed both when running an ECall, and
// resolving a WAMR native symbol.
extern std::unordered_map<uint32_t, std::shared_ptr<wasm::EnclaveWasmModule>>
  moduleMap;
extern std::mutex moduleMapMutex;

// Return the EnclaveWasmModule that is executing in a given WASM execution
// environment. This method relies on `wasm_exec_env_t` having a `module_inst`
// property, pointint to the instantiated module.
std::shared_ptr<wasm::EnclaveWasmModule> getExecutingEnclaveWasmModule(
  wasm_exec_env_t execEnv);
}

// Given that we can not throw exceptions, we wrap the call to the method to
// get the executing enclave with a check for success. This macro is meant to
// be used in the implementation of native symbols, where returning 1 is
// interpreted as a failure.
#define GET_EXECUTING_MODULE_AND_CHECK(execEnv)                                \
    std::shared_ptr<wasm::EnclaveWasmModule> module =                          \
      wasm::getExecutingEnclaveWasmModule(execEnv);                            \
    if (module == nullptr) {                                                   \
        ocallLogError(                                                         \
          "Error linking execution environment to registered modules");        \
        return 1;                                                              \
    }
