#include <enclave/inside/EnclaveWasmModule.h>
#include <enclave/inside/native.h>
#include <enclave/inside/ocalls.h>

#include <memory>

#include <sgx_report.h>
#include <sgx_trts.h>
#include <sgx_utils.h>

// Implementation of the ECalls API
extern "C"
{
    faasm_sgx_status_t ecallCreateReport(const sgx_target_info_t* qeTarget,
                                         const sgx_report_data_t* heldData,
                                         sgx_report_t* report)
    {
        sgx_status_t sgxReturnValue =
          sgx_create_report(qeTarget, heldData, report);

        switch (sgxReturnValue) {
            case SGX_ERROR_INVALID_PARAMETER:
                return FAASM_SGX_INVALID_PTR;
            case SGX_SUCCESS:
                return FAASM_SGX_SUCCESS;
            default:
                return FAASM_SGX_GENERATE_REPORT_FAILED;
        }
    }

    faasm_sgx_status_t ecallInitWamr(void)
    {
        // Initialise WAMR once for all modules
        ocallLogDebug("Initialising WAMR runtime");
        if (!wasm::EnclaveWasmModule::initialiseWAMRGlobally()) {
            ocallLogError("Error initialising WAMR globally");
            return FAASM_SGX_WAMR_RTE_INIT_FAILED;
        }

        // Register native symbols
        sgx::initialiseSGXWAMRNatives();

        return FAASM_SGX_SUCCESS;
    }

    faasm_sgx_status_t ecallLoadModule(void* wasmOpCodePtr,
                                       uint32_t wasmOpCodeSize,
                                       uint32_t faasletId)
    {
        // Check if passed wasm opcode size or wasm opcode ptr is zero
        if (!wasmOpCodeSize) {
            return FAASM_SGX_INVALID_OPCODE_SIZE;
        }
        if (!wasmOpCodePtr) {
            return FAASM_SGX_INVALID_PTR;
        }

        // Add module for faaslet to the module map
        {
            std::unique_lock<std::mutex> lock(wasm::moduleMapMutex);
            if (wasm::moduleMap.find(faasletId) != wasm::moduleMap.end()) {
                ocallLogError("Faaslet is already bound to a module.");
                return FAASM_SGX_WAMR_MODULE_LOAD_FAILED;
            }

            wasm::moduleMap[faasletId] =
              std::make_shared<wasm::EnclaveWasmModule>();
            if (!wasm::moduleMap[faasletId]->loadWasm(wasmOpCodePtr,
                                                      wasmOpCodeSize)) {
                ocallLogError("Error loading WASM to module");
                return FAASM_SGX_WAMR_MODULE_LOAD_FAILED;
            }
        }

        return FAASM_SGX_SUCCESS;
    }

    faasm_sgx_status_t ecallUnloadModule(uint32_t faasletId)
    {
        std::unique_lock<std::mutex> lock(wasm::moduleMapMutex);
        if (wasm::moduleMap.find(faasletId) == wasm::moduleMap.end()) {
            ocallLogError("Faaslet not bound to any module.");
            return FAASM_SGX_WAMR_MODULE_NOT_BOUND;
        }

        // Erase will call the underlying destructor for the module
        wasm::moduleMap.erase(faasletId);

        return FAASM_SGX_SUCCESS;
    }

    faasm_sgx_status_t ecallCallFunction(uint32_t faasletId,
                                         uint32_t argc,
                                         char** argv)
    {
        std::shared_ptr<wasm::EnclaveWasmModule> module = nullptr;

        // Acquire a lock just to get the module
        {
            std::unique_lock<std::mutex> lock(wasm::moduleMapMutex);
            if (wasm::moduleMap.find(faasletId) == wasm::moduleMap.end()) {
                ocallLogError("Faaslet not bound to any module.");
                return FAASM_SGX_WAMR_MODULE_NOT_BOUND;
            }

            module = wasm::moduleMap[faasletId];
        }

        // Call the function without a lock on the module map, to allow for
        // chaining on the same enclave
        if (!module->callFunction(argc, argv)) {
            ocallLogError("Error trying to call function");
            return FAASM_SGX_WAMR_FUNCTION_UNABLE_TO_CALL;
        }

        return FAASM_SGX_SUCCESS;
    }
}
