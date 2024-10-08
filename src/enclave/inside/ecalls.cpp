#include <enclave/inside/EnclaveWasmModule.h>
#include <enclave/inside/native.h>
#include <enclave/inside/ocalls.h>
#include <enclave/inside/tests.h>

#include <memory>

#include <sgx_report.h>
#include <sgx_trts.h>
#include <sgx_utils.h>

// Implementation of the ECalls API
extern "C"
{
    // TODO: probably we want to handle all this logic from inside the enclave
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
        SPDLOG_DEBUG_SGX("Initialising WAMR runtime");
        if (!wasm::EnclaveWasmModule::initialiseWAMRGlobally()) {
            SPDLOG_ERROR_SGX("Error initialising WAMR globally");
            return FAASM_SGX_WAMR_RTE_INIT_FAILED;
        }

        return FAASM_SGX_SUCCESS;
    }

    faasm_sgx_status_t ecallReset()
    {
        if (wasm::enclaveWasmModule == nullptr) {
            ocallLogError("Faaslet not bound to any module!");
            return FAASM_SGX_WAMR_MODULE_NOT_BOUND;
        }

        wasm::enclaveWasmModule->reset();

        return FAASM_SGX_SUCCESS;
    }

    faasm_sgx_status_t ecallDoBindToFunction(const char* user,
                                             const char* func,
                                             void* wasmOpCodePtr,
                                             uint32_t wasmOpCodeSize,
                                             uint32_t faasletId)
    {
        SPDLOG_DEBUG_SGX("Binding to %s/%s (%i)", user, func, faasletId);

        // Check if passed wasm opcode size or wasm opcode ptr is zero
        if (!wasmOpCodeSize) {
            return FAASM_SGX_INVALID_OPCODE_SIZE;
        }
        if (!wasmOpCodePtr) {
            return FAASM_SGX_INVALID_PTR;
        }

        wasm::enclaveWasmModule =
          std::make_shared<wasm::EnclaveWasmModule>(user, func);

        if (!wasm::enclaveWasmModule->doBindToFunction(wasmOpCodePtr,
                                                       wasmOpCodeSize)) {
            SPDLOG_ERROR_SGX(
              "Error binding SGX-WAMR module to %s/%s", user, func);
            return FAASM_SGX_WAMR_MODULE_LOAD_FAILED;
        }

        return FAASM_SGX_SUCCESS;
    }

    faasm_sgx_status_t ecallDestroyModule(uint32_t faasletId)
    {
        if (wasm::enclaveWasmModule == nullptr) {
            ocallLogError("Faaslet not bound to any module!");
            return FAASM_SGX_WAMR_MODULE_NOT_BOUND;
        }

        // Call the destructor on the module
        wasm::enclaveWasmModule.reset();

        return FAASM_SGX_SUCCESS;
    }

    faasm_sgx_status_t ecallCallFunction(uint32_t faasletId,
                                         uint32_t argc,
                                         char** argv)
    {
        if (wasm::enclaveWasmModule == nullptr) {
            ocallLogError("Faaslet not bound to any module!");
            return FAASM_SGX_WAMR_MODULE_NOT_BOUND;
        }

        // Call the function without a lock on the module map, to allow for
        // chaining on the same enclave
        uint32_t returnValue =
          wasm::enclaveWasmModule->callFunction(argc, argv);
        if (returnValue != 0) {
            SPDLOG_ERROR_SGX("Error trying to call function. Return value: %i",
                             returnValue);
            return FAASM_SGX_WAMR_FUNCTION_UNABLE_TO_CALL;
        }

        return FAASM_SGX_SUCCESS;
    }

    faasm_sgx_status_t ecallCopyDataIn(uint8_t* buffer, uint32_t bufferSize)
    {
        if (wasm::enclaveWasmModule == nullptr) {
            ocallLogError("Faaslet not bound to any module!");
            return FAASM_SGX_WAMR_MODULE_NOT_BOUND;
        }

        // TODO: this ECall is triggered by the untrsuted host, so we should
        // sanitize that we are not malloc-ing something ridiculous. Ideally
        // we should be able to know the data we expect to receive before
        // hand, and double-check it here
        wasm::enclaveWasmModule->dataXferPtr = (uint8_t*)malloc(bufferSize);
        memcpy(wasm::enclaveWasmModule->dataXferPtr, buffer, bufferSize);
        wasm::enclaveWasmModule->dataXferSize = bufferSize;

        return FAASM_SGX_SUCCESS;
    }

    faasm_sgx_status_t ecallRunInternalTest(const char* testCase)
    {
        if (wasm::enclaveWasmModule == nullptr) {
            ocallLogError("Faaslet not bound to any module!");
            return FAASM_SGX_WAMR_MODULE_NOT_BOUND;
        }

        SPDLOG_DEBUG_SGX("Running test: %s", testCase);
        if (std::string(testCase) == "hello-world") {
            return tests::testHello();
        }

        // Unreachable
        return FAASM_SGX_INTERNAL_TEST_ERROR;
    }
}
