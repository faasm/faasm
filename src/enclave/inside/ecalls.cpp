#include <enclave/inside/EnclaveWasmModule.h>
#include <enclave/inside/native.h>
#include <enclave/inside/ocalls.h>

#include <memory>
#include <mutex>
#include <unordered_map>

// Implementation of the ECalls API
extern "C"
{
    static std::unordered_map<uint32_t, std::shared_ptr<wasm::EnclaveWasmModule>> moduleMap;
    static std::mutex moduleMapMutex;

    faasm_sgx_status_t faasm_sgx_enclave_init_wamr(void)
    {
        // TODO - implement proper logging
        ocall_faasm_log_debug("Initialising WAMR runtime");
        // Initialise WAMR once for all modules
        if (!wasm::EnclaveWasmModule::initialiseWAMRGlobally()) {
            ocall_faasm_log_error("Error initialising WAMR globally");
            return FAASM_SGX_WAMR_RTE_INIT_FAILED;
        }

        // Register native symbols
        sgx::initialiseSGXWAMRNatives();

        return FAASM_SGX_SUCCESS;
    }

    faasm_sgx_status_t faasm_sgx_enclave_load_module(
      void* wasm_opcode_ptr,
      uint32_t wasm_opcode_size,
      uint32_t faaslet_id)
    {
        // Check if passed wasm opcode size or wasm opcode ptr is zero
        if (!wasm_opcode_size) {
            return FAASM_SGX_INVALID_OPCODE_SIZE;
        }
        if (!wasm_opcode_ptr) {
            return FAASM_SGX_INVALID_PTR;
        }

        // Add module for faaslet to the module map
        {
            std::unique_lock<std::mutex> lock(moduleMapMutex);
            if (moduleMap.find(faaslet_id) != moduleMap.end()) {
                ocall_faasm_log_error("Faaslet is already bound to a module.");
                return FAASM_SGX_WAMR_MODULE_LOAD_FAILED;
            }

            moduleMap[faaslet_id] = std::make_shared<wasm::EnclaveWasmModule>();
            if (!moduleMap[faaslet_id]->loadWasm(wasm_opcode_ptr, wasm_opcode_size)) {
                ocall_faasm_log_error("Error loading WASM to module");
                return FAASM_SGX_WAMR_MODULE_LOAD_FAILED;
            }
        }

        return FAASM_SGX_SUCCESS;
    }

    faasm_sgx_status_t faasm_sgx_enclave_unload_module(uint32_t faaslet_id)
    {
        std::unique_lock<std::mutex> lock(moduleMapMutex);
        if (moduleMap.find(faaslet_id) == moduleMap.end()) {
            ocall_faasm_log_error("Faaslet not bound to any module.");
            return FAASM_SGX_WAMR_MODULE_NOT_BOUND;
        }

        // Erase will call the underlying destructor for the module
        moduleMap.erase(faaslet_id);

        return FAASM_SGX_SUCCESS;
    }

    faasm_sgx_status_t faasm_sgx_enclave_call_function(uint32_t faaslet_id)
    {
        std::shared_ptr<wasm::EnclaveWasmModule> module = nullptr;

        // Acquire a lock just to get the module
        {
            std::unique_lock<std::mutex> lock(moduleMapMutex);
            if (moduleMap.find(faaslet_id) == moduleMap.end()) {
                ocall_faasm_log_error("Faaslet not bound to any module.");
                return FAASM_SGX_WAMR_MODULE_NOT_BOUND;
            }

            module = moduleMap[faaslet_id];
        }

        // Call the function without a lock on the module map, to allow for
        // chaining on the same enclave
        if (!module->callFunction()) {
            ocall_faasm_log_error("Error trying to call function");
            return FAASM_SGX_WAMR_FUNCTION_UNABLE_TO_CALL;
        }

        return FAASM_SGX_SUCCESS;
    }
}
