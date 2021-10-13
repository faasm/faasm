#include <sgx/attestation.h>
#include <sgx/enclave_config.h>
#include <sgx/enclave_types.h>
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

#ifdef FAASM_SGX_DEBUG
    // Print functions
    typedef void (*os_print_function_t)(const char* msg);
    extern void os_set_print_function(os_print_function_t pf);
    extern sgx_status_t SGX_CDECL ocall_printf(const char* msg);
    extern int os_printf(const char* message, ...);
#endif

    // Print function wrapper
    void SGX_DEBUG_LOG(const char* message)
    {
#ifdef FAASM_SGX_DEBUG
        ocall_printf(message);
#else
        ;
#endif
    }

    static sgx::ModuleStore wamrModules(10);

    static uint8_t wamrHeapBuffer[SGX_WAMR_HEAP_SIZE];

    // Set up the WAMR runtime, and initialise all enclave-related
    // variables. Currently, this happens _once_ per Faasm instance. This is,
    // we only run one enclave per Faasm instance.
    faasm_sgx_status_t enclaveInitWamr(void)
    {
        os_set_print_function((os_print_function_t)SGX_DEBUG_LOG);

        // Initialise the WAMR runtime
        RuntimeInitArgs wamr_rte_args;
        memset(&wamr_rte_args, 0x0, sizeof(wamr_rte_args));
        wamr_rte_args.mem_alloc_type = Alloc_With_Pool;
        wamr_rte_args.mem_alloc_option.pool.heap_buf = (void*)wamrHeapBuffer;
        wamr_rte_args.mem_alloc_option.pool.heap_size = sizeof(wamrHeapBuffer);

        if (!wasm_runtime_full_init(&wamr_rte_args)) {
            return FAASM_SGX_WAMR_RTE_INIT_FAILED;
        }

        // Set up native symbols
        sgx::initialiseSGXWAMRNatives();

        return FAASM_SGX_SUCCESS;
    }

    // Load the provided web assembly module to the enclave's runtime
    faasm_sgx_status_t enclaveLoadModule(const void* wasmOpCodePtr,
                                         const uint32_t wasmOpCodeSize,
                                         uint32_t* moduleSlot)
    {
        char errorBuffer[SGX_ERROR_BUFFER_SIZE];

        // Check if passed wasm opcode size or wasm opcode ptr is zero
        if (!wasmOpCodeSize) {
            return FAASM_SGX_INVALID_OPCODE_SIZE;
        }
        if (wasmOpCodePtr == NULL) {
            return FAASM_SGX_INVALID_PTR;
        }
        uint8_t* wasmBytes = (uint8_t*)calloc(wasmOpCodeSize, sizeof(uint8_t));
        memcpy(wasmBytes, wasmOpCodePtr, wasmOpCodeSize);
        *moduleSlot = wamrModules.store(wasmOpCodePtr, wasmOpCodeSize);

        // Load the WASM module
        wamrModuleHandler.wasmModule = wasm_runtime_load(
          wasmBytes, wasmOpCodeSize, errorBuffer, sizeof(errorBuffer));

        if (wamrModuleHandler.wasmModule == NULL) {
            SGX_DEBUG_LOG(errorBuffer);
            return FAASM_SGX_WAMR_MODULE_LOAD_FAILED;
        }

        // Instantiate the WASM module
        wamrModuleHandler.moduleInstance =
          wasm_runtime_instantiate(wamrModuleHandler.wasmModule,
                                   (uint32_t)SGX_INSTANCE_STACK_SIZE,
                                   (uint32_t)SGX_INSTANCE_HEAP_SIZE,
                                   errorBuffer,
                                   sizeof(errorBuffer));

        if (wamrModuleHandler.moduleInstance == NULL) {
            wasm_runtime_unload(wamrModuleHandler.wasmModule);
            SGX_DEBUG_LOG(errorBuffer);
            return FAASM_SGX_WAMR_MODULE_INSTANTIATION_FAILED;
        }

        return FAASM_SGX_SUCCESS;
    }

    faasm_sgx_status_t enclaveUnloadModule(const uint32_t thread_id)
    {
        // Unload the module and release the TCS slot
        wasm_runtime_deinstantiate(wamrModuleHandler.moduleInstance);
        wasm_runtime_unload(wamrModuleHandler.wasmModule);

        return FAASM_SGX_SUCCESS;
    }

    // Execute the main function
    faasm_sgx_status_t enclaveCallFunction()
    {
        // Get function to execute
        WASMFunctionInstanceCommon* func = wasm_runtime_lookup_function(
          wamrModuleHandler.moduleInstance, WASM_ENTRY_FUNC, NULL);

        if (func == NULL) {
            SGX_DEBUG_LOG("Failed to instantiate WAMR function in enclave");
            return FAASM_SGX_WAMR_FUNCTION_NOT_FOUND;
        }

        std::vector<uint32_t> argv = { 0 };

        // Invoke the function
        bool success = aot_create_exec_env_and_call_function(
          reinterpret_cast<AOTModuleInstance*>(
            wamrModuleHandler.moduleInstance),
          reinterpret_cast<AOTFunctionInstance*>(func),
          0x0,
          argv.data());

        if (!success) {
            // First, check if the _FAASM_SGX_ERROR_PREFIX is set
            // If so, then obtain and return the faasm_sgx_status_t error code
            // If not, just print the exception and return the matching
            // Faasm-SGX error code
            if (!memcmp(((AOTModuleInstance*)wamrModuleHandler.moduleInstance)
                          ->cur_exception,
                        _FAASM_SGX_ERROR_PREFIX,
                        sizeof(_FAASM_SGX_ERROR_PREFIX))) {

                return *((faasm_sgx_status_t*)&(
                  ((AOTModuleInstance*)wamrModuleHandler.moduleInstance)
                    ->cur_exception[sizeof(_FAASM_SGX_ERROR_PREFIX)]));
            }

            SGX_DEBUG_LOG(((AOTModuleInstance*)wamrModuleHandler.moduleInstance)
                            ->cur_exception);
            return FAASM_SGX_WAMR_FUNCTION_UNABLE_TO_CALL;
        }

        return FAASM_SGX_SUCCESS;
    }
}
