#include <sgx/ModuleStore.h>
#include <sgx/attestation.h>
#include <sgx/enclave_config.h>
#include <sgx/error.h>
#include <sgx/native.h>
#include <sgx/ocalls.h>
#include <sgx/rw_lock.h>

#include <iwasm/include/wasm_export.h>
#include <libcxx/cstdlib>
#include <sgx.h>
#include <sgx_defs.h>
#include <sgx_thread.h>
#include <tlibc/mbusafecrt.h>
#include <vector>

#include <iwasm/aot/aot_runtime.h>

#define WASM_CTORS_FUNC_NAME "__wasm_call_ctors"
#define WASM_ENTRY_FUNC "_start"

// Implementation of the exposed Ecalls API. The code in this file is meant to
// run _inside_ the enclave, thus is the only bit of code that actually
// interacts with the WAMR runtime.
extern "C"
{
#ifndef NDEBUG
    // Print functions
    typedef void (*os_print_function_t)(const char* msg);
    extern void os_set_print_function(os_print_function_t pf);
    extern sgx_status_t SGX_CDECL ocall_printf(const char* msg);
    extern int os_printf(const char* message, ...);
#endif

    // Print function wrapper
    void SGX_DEBUG_LOG(const char* fmt, ...)
    {
#ifndef NDEBUG
        va_list args;
        va_start(args, fmt);
        char message[SGX_MSG_BUFFER_SIZE];
        vsnprintf(message, SGX_MSG_BUFFER_SIZE, fmt, args);
        va_end(args);
        ocall_printf(message);
#else
        ;
#endif
    }

    static sgx::ModuleStore wamrModules;
    static thread_local std::shared_ptr<sgx::WamrModuleHandle> moduleHandle = nullptr;

    static uint8_t wamrHeapBuffer[SGX_WAMR_HEAP_SIZE];

    // Set up the WAMR runtime, and initialise all enclave-related
    // variables. Currently, this happens _once_ per Faasm instance. This is,
    // we only run one enclave per Faasm instance.
    faasm_sgx_status_t enclaveInitWamr(void)
    {
        // Initialise the WAMR runtime
        RuntimeInitArgs wamrRuntimeArgs;
        memset(&wamrRuntimeArgs, 0x0, sizeof(wamrRuntimeArgs));
        wamrRuntimeArgs.mem_alloc_type = Alloc_With_Pool;
        wamrRuntimeArgs.mem_alloc_option.pool.heap_buf = (void*)wamrHeapBuffer;
        wamrRuntimeArgs.mem_alloc_option.pool.heap_size =
          sizeof(wamrHeapBuffer);

        if (!wasm_runtime_full_init(&wamrRuntimeArgs)) {
            return FAASM_SGX_WAMR_RTE_INIT_FAILED;
        }

        // Set up native symbols
        sgx::initialiseSGXWAMRNatives();

        return FAASM_SGX_SUCCESS;
    }

    // Load the provided web assembly module to the enclave's runtime
    faasm_sgx_status_t enclaveLoadModule(const char* funcStr,
                                         const void* wasmOpCodePtr,
                                         uint32_t wasmOpCodeSize)
    {
        char errorBuffer[SGX_ERROR_BUFFER_SIZE];

        // Check if passed wasm opcode size or wasm opcode ptr is zero
        if (wasmOpCodeSize == 0) {
            return FAASM_SGX_INVALID_OPCODE_SIZE;
        }
        if (wasmOpCodePtr == nullptr) {
            return FAASM_SGX_INVALID_PTR;
        }

        // Load the WASM module to the module store
        // std::shared_ptr<sgx::WamrModuleHandle> moduleHandle =
        moduleHandle = wamrModules.store(funcStr, wasmOpCodePtr, wasmOpCodeSize);
        if (moduleHandle == nullptr) {
            ocall_faasm_log_error("Can't load module to the SGX store");
            return FAASM_SGX_MODULE_STORE_FAILED;
        }
        SGX_DEBUG_LOG("Loaded module %s in the module store", funcStr);

        // Load the WASM module to the WAMR runtime
        moduleHandle->wasmModule =
          wasm_runtime_load(moduleHandle->wasmOpCodePtr,
                            moduleHandle->wasmOpCodeSize,
                            errorBuffer,
                            sizeof(errorBuffer));
        if (moduleHandle->wasmModule == nullptr) {
            ocall_faasm_log_error(errorBuffer);
            return FAASM_SGX_WAMR_MODULE_LOAD_FAILED;
        }

        // Instantiate the WASM module
        moduleHandle->moduleInstance =
          wasm_runtime_instantiate(moduleHandle->wasmModule,
                                   (uint32_t)SGX_INSTANCE_STACK_SIZE,
                                   (uint32_t)SGX_INSTANCE_HEAP_SIZE,
                                   errorBuffer,
                                   sizeof(errorBuffer));

        if (moduleHandle->moduleInstance == NULL) {
            wasm_runtime_unload(moduleHandle->wasmModule);
            ocall_faasm_log_error(errorBuffer);
            return FAASM_SGX_WAMR_MODULE_INSTANTIATION_FAILED;
        }
        SGX_DEBUG_LOG("Loaded WASM module (%s) to the runtime", funcStr);

        return FAASM_SGX_SUCCESS;
    }

    faasm_sgx_status_t enclaveUnloadModule(const char* funcStr)
    {
        /* jaja
        std::shared_ptr<sgx::WamrModuleHandle> moduleHandle =
          wamrModules.get(funcStr);
          */
        if (moduleHandle == nullptr) {
            SGX_DEBUG_LOG("Module slot is not set, skipping unload");
            return FAASM_SGX_SUCCESS;
        }

        // Unload the module and release the TCS slot
        wasm_runtime_deinstantiate(moduleHandle->moduleInstance);
        wasm_runtime_unload(moduleHandle->wasmModule);

        // Clean the module slot
        bool success = wamrModules.clear(funcStr);
        if (!success) {
            ocall_faasm_log_error("Error clearing module from module store");
            return FAASM_SGX_MODULE_STORE_CLEAR_FAILED;
        }

        moduleHandle = nullptr;

        return FAASM_SGX_SUCCESS;
    }

    // Execute the main function
    faasm_sgx_status_t enclaveCallFunction(const char* key)
    {
        /*
        std::shared_ptr<sgx::WamrModuleHandle> moduleHandle =
          wamrModules.get(key);
        */
        if (moduleHandle == nullptr) {
            ocall_faasm_log_error("SGX-WAMR module not loaded. Has module been bound?");
            return FAASM_SGX_INVALID_PTR;
        }

        // Get function to execute
        WASMFunctionInstanceCommon* func = wasm_runtime_lookup_function(
          moduleHandle->moduleInstance, WASM_ENTRY_FUNC, NULL);

        if (func == nullptr) {
            ocall_faasm_log_error(
              "Failed to instantiate WAMR function in enclave");
            return FAASM_SGX_WAMR_FUNCTION_NOT_FOUND;
        }

        std::vector<uint32_t> argv = { 0 };

        // Invoke the function
        bool success = aot_create_exec_env_and_call_function(
          reinterpret_cast<AOTModuleInstance*>(moduleHandle->moduleInstance),
          reinterpret_cast<AOTFunctionInstance*>(func),
          0x0,
          argv.data());

        if (!success) {
            // First, check if the _FAASM_SGX_ERROR_PREFIX is set
            // If so, then obtain and return the faasm_sgx_status_t error code
            // If not, just print the exception and return the matching
            // Faasm-SGX error code
            if (!memcmp(((AOTModuleInstance*)moduleHandle->moduleInstance)
                          ->cur_exception,
                        _FAASM_SGX_ERROR_PREFIX,
                        sizeof(_FAASM_SGX_ERROR_PREFIX))) {

                return *((faasm_sgx_status_t*)&(
                  ((AOTModuleInstance*)moduleHandle->moduleInstance)
                    ->cur_exception[sizeof(_FAASM_SGX_ERROR_PREFIX)]));
            }

            ocall_faasm_log_error(
              ((AOTModuleInstance*)moduleHandle->moduleInstance)
                ->cur_exception);
            return FAASM_SGX_WAMR_FUNCTION_UNABLE_TO_CALL;
        }

        return FAASM_SGX_SUCCESS;
    }
}

// TODO - waatt
extern "C" int __cxa_thread_atexit(void (*dtor)(void *), void *obj, void *dso_symbol) { return 0; }
