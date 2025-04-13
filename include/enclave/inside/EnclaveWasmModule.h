#pragma once

#include <enclave/inside/crypto/types.h>
#include <wamr/WAMRModuleMixin.h>

#include <iwasm/aot/aot_runtime.h>
#include <wasm_runtime_common.h>

#include <vector>

namespace wasm {

/*
 * Abstraction around a WebAssembly module running inside an SGX enclave with
 * the WAMR runtime. An EnclaveWasmModule is bound to a unique user/function
 * pair, and there is only one module inside an enclave.
 * */
class EnclaveWasmModule : public WAMRModuleMixin<EnclaveWasmModule>
{
  public:
    static bool initialiseWAMRGlobally();

    EnclaveWasmModule();
    ~EnclaveWasmModule();

    // Called after every execution, leaves the module ready to execute another
    // instance of the _same_ function.
    bool reset();

    bool doBindToFunction(const char* user,
                          const char* function,
                          uint8_t* wasmOpCodePtr,
                          uint32_t wasmOpCodeSize);

    uint32_t callFunction(uint32_t argcIn, char** argvIn);

    // TODO: remove duplication with WAMRWasmModule
    int executeWasmFunction(const std::string& funcName);

    WASMModuleInstanceCommon* getModuleInstance();

    bool isBound() const { return _isBound; };

    std::string getBoundUser() const { return user; }

    std::string getBoundFunction() const { return function; }

    void validateNativePointer(void* nativePtr, int size);

    // ----- Exception handling -----
    void doThrowException(std::exception& exc) const;

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

    uint32_t mmapMemory(size_t nBytes);

    uint32_t mmapFile(uint32_t wasmFd, size_t length);

    void unmapMemory(uint32_t offset, size_t nBytes);

    // There are two ways to transfer data into the enclave. Either as an [in]
    // buffer as an argument of an ECall or an [out] buffer as an argument of
    // an OCall. The OCall situation is more common, as it is the enclave
    // requesting information from the outside using an OCall. Unfortunately,
    // OCall arguments are stored in the untrusted application's stack, meanng
    // that we are limited to whatever the stack size is (usually a few KBs).
    // On the other hand, ECall arguments are heap-allocated in the enclave
    // memory, allowing arbitrarily large data transfers. We use by default the
    // OCall mechanism, and fall-back to the ECall one if needed. The following
    // two variables are used to stage data that is transferred in an OCall +
    // ECall mechanism
    uint8_t* dataXferPtr = nullptr;
    size_t dataXferSize = 0;

    uint8_t* dataXferAuxPtr = nullptr;
    size_t dataXferAuxSize = 0;

    // ---- Crypto management ----

    FaasmPublicKey getPubKey() { return publicKey; }
    FaasmPrivateKey getPrivKey() { return privateKey; }
    FaasmKeyContext getKeyContext() { return keyContext; }

    // SGX
    std::shared_ptr<sgx_report_t> cachedSgxReport = nullptr;

    bool isAcclessEnabled() const { return this->acclessEnabled; }

    void setAcclessMode(bool acclessEnabled)
    {
        this->acclessEnabled = acclessEnabled;
    }

  private:
    char errorBuffer[WAMR_ERROR_BUFFER_SIZE];

    WASMModuleCommon* wasmModule;
    WASMModuleInstanceCommon* moduleInstance;

    // Heap pointer to the bytes for the WASM module. These are the bytes that
    // we fit to wasm_runtime_load, and they need to be available
    // until we call wasm_runtime_unload
    uint8_t* wasmModuleBytes = nullptr;
    int32_t wasmModuleBytesSize = 0;

    std::string user;
    std::string function;

    bool _isBound = false;
    bool bindInternal();

    // Argc/argv
    uint32_t argc;
    std::vector<std::string> argv;
    size_t argvBufferSize;

    void prepareArgcArgv(uint32_t argcIn, char** argvIn);

    // Memory management
    uint32_t currentBrk = 0;

    // Cypto
    FaasmKeyContext keyContext;
    FaasmPrivateKey privateKey;
    FaasmPublicKey publicKey;

    bool acclessEnabled;
};

// Data structure to keep track of the module currently loaded in the enclave.
// Needs to have external definition as it needs to be accessed both when
// running an ECall and resolving a WAMR native symbol
// extern std::shared_ptr<wasm::EnclaveWasmModule> enclaveWasmModule;
extern "C" EnclaveWasmModule* getExecutingEnclaveWasmModule();

// Return the EnclaveWasmModule that is executing in a given WASM execution
// environment. This method relies on `wasm_exec_env_t` having a `module_inst`
// property, pointint to the instantiated module.
EnclaveWasmModule* getExecutingEnclaveWasmModule(wasm_exec_env_t execEnv);
}

// Given that we can not throw exceptions, we wrap the call to the method to
// get the executing enclave with a check for success. This macro is meant to
// be used in the implementation of native symbols, where returning 1 is
// interpreted as a failure.
#define GET_EXECUTING_MODULE_AND_CHECK(execEnv)                                \
    auto* module = wasm::getExecutingEnclaveWasmModule(execEnv);               \
    if (module == nullptr) {                                                   \
        ocallLogError(                                                         \
          "Error linking execution environment to registered modules");        \
        return 1;                                                              \
    }
