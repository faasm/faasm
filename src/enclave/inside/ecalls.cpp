#include <enclave/inside/EnclaveWasmModule.h>
#include <enclave/inside/native.h>
#include <enclave/inside/ocalls.h>

#include <memory>

#include "wasm_export.h"
#include "bh_platform.h"

#include <sgx_report.h>
#include <sgx_trts.h>
#include <sgx_utils.h>

// TODO: move elsewhere
#define REQUIRE(cond) \
    if (!(cond)) { \
        SPDLOG_ERROR_SGX("Error testing assertion!"); \
        return FAASM_SGX_INTERNAL_TEST_ERROR; \
    } \

// Implementation of the ECalls API
extern "C"
{
    typedef void (*os_print_function_t)(const char* message);
    extern void os_set_print_function(os_print_function_t pf);

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
        os_set_print_function(ocallLogWamr);

        // Initialise WAMR once for all modules
        ocallLogDebug("Initialising WAMR runtime");
        if (!wasm::EnclaveWasmModule::initialiseWAMRGlobally()) {
            ocallLogError("Error initialising WAMR globally");
            return FAASM_SGX_WAMR_RTE_INIT_FAILED;
        }

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
                SPDLOG_ERROR_SGX("Faaslet not bound to any module.");
                return FAASM_SGX_WAMR_MODULE_NOT_BOUND;
            }

            module = wasm::moduleMap[faasletId];
        }

        // Call the function without a lock on the module map, to allow for
        // chaining on the same enclave
        uint32_t returnValue = module->callFunction(argc, argv);
        if (returnValue != 0) {
            SPDLOG_ERROR_SGX("Error trying to call function. Return value: %i", returnValue);
            return FAASM_SGX_WAMR_FUNCTION_UNABLE_TO_CALL;
        }

        return FAASM_SGX_SUCCESS;
    }

    // TODO: switch on the test case
    faasm_sgx_status_t ecallRunInternalTest(uint32_t faasletId,
                                            const char* testCase)
    {
        std::shared_ptr<wasm::EnclaveWasmModule> module = nullptr;

        // Acquire a lock just to get the module
        {
            std::unique_lock<std::mutex> lock(wasm::moduleMapMutex);
            if (wasm::moduleMap.find(faasletId) == wasm::moduleMap.end()) {
                SPDLOG_ERROR_SGX("Faaslet not bound to any module.");
                return FAASM_SGX_WAMR_MODULE_NOT_BOUND;
            }

            module = wasm::moduleMap[faasletId];
        }

        // TODO: probably move the function body elsewhere
        if (std::string(testCase) == "sbrk") {
            SPDLOG_DEBUG_SGX("Running test: %s", testCase);

            size_t initialSize = module->getMemorySizeBytes();
            REQUIRE(module->getCurrentBrk() == initialSize);

            uint32_t growA = 5 * WASM_BYTES_PER_PAGE;
            uint32_t growB = 20 * WASM_BYTES_PER_PAGE;

            module->growMemory(growA);
            size_t sizeA = module->getMemorySizeBytes();
            REQUIRE(sizeA > initialSize);
            REQUIRE(sizeA == initialSize + growA);
            REQUIRE(module->getCurrentBrk() == sizeA);

            module->growMemory(growB);
            size_t sizeB = module->getMemorySizeBytes();
            REQUIRE(sizeB > initialSize + growA);
            REQUIRE(sizeB == initialSize + growA + growB);
            REQUIRE(module->getCurrentBrk() == sizeB);

            // Lastly, run the function to verify that the heap has not been
            // corrupted
            char **argvIn = (char *[]){"function.wasm"};
            REQUIRE(module->callFunction(1, argvIn) == 0);
        }

        return FAASM_SGX_SUCCESS;
    }
}
