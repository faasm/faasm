#include <enclave/inside/EnclaveWasmModule.h>
#include <enclave/inside/native.h>
#include <enclave/inside/ocalls.h>
#include <enclave/inside/tests.h>

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

    faasm_sgx_status_t ecallInitWamr()
    {
        // Initialise WAMR once for all modules
        SPDLOG_DEBUG_SGX("Initialising SGX-WAMR runtime");
        if (!wasm::EnclaveWasmModule::initialiseWAMRGlobally()) {
            SPDLOG_ERROR_SGX("Error initialising WAMR globally");
            return FAASM_SGX_WAMR_RTE_INIT_FAILED;
        }

        return FAASM_SGX_SUCCESS;
    }

    faasm_sgx_status_t ecallReset()
    {
        auto* enclaveWasmModule = wasm::getExecutingEnclaveWasmModule();
        if (enclaveWasmModule == nullptr) {
            ocallLogError("Faaslet not bound to any module!");
            return FAASM_SGX_WAMR_MODULE_NOT_BOUND;
        }

        enclaveWasmModule->reset();

        return FAASM_SGX_SUCCESS;
    }

    faasm_sgx_status_t ecallDoBindToFunction(const char* user,
                                             const char* func,
                                             void* wasmBytes,
                                             uint32_t wasmBytesSize,
                                             bool acclessEnabled)
    {
        SPDLOG_DEBUG_SGX(
          "Binding to %s/%s (accless: %i)", user, func, acclessEnabled);

        // Check if passed wasm opcode size or wasm opcode ptr is zero
        if (!wasmBytesSize) {
            return FAASM_SGX_INVALID_OPCODE_SIZE;
        }
        if (!wasmBytes) {
            return FAASM_SGX_INVALID_PTR;
        }

        auto* enclaveWasmModule = wasm::getExecutingEnclaveWasmModule();
        if (!enclaveWasmModule->doBindToFunction(
              user, func, (uint8_t*)wasmBytes, wasmBytesSize)) {
            SPDLOG_ERROR_SGX(
              "Error binding SGX-WAMR module to %s/%s", user, func);
            return FAASM_SGX_WAMR_MODULE_LOAD_FAILED;
        }

        enclaveWasmModule->setAcclessMode(acclessEnabled);

        return FAASM_SGX_SUCCESS;
    }

    faasm_sgx_status_t ecallDestroyModule(uint32_t faasletId)
    {
        auto* enclaveWasmModule = wasm::getExecutingEnclaveWasmModule();
        if (enclaveWasmModule == nullptr) {
            ocallLogError("Faaslet not bound to any module!");
            return FAASM_SGX_WAMR_MODULE_NOT_BOUND;
        }

        // Call the destructor on the module
        enclaveWasmModule->reset();

        return FAASM_SGX_SUCCESS;
    }

    faasm_sgx_status_t ecallCallFunction(uint32_t faasletId,
                                         uint32_t argc,
                                         char** argv)
    {
        auto* enclaveWasmModule = wasm::getExecutingEnclaveWasmModule();
        if (enclaveWasmModule == nullptr) {
            ocallLogError("Faaslet not bound to any module!");
            return FAASM_SGX_WAMR_MODULE_NOT_BOUND;
        }

        // Call the function without a lock on the module map, to allow for
        // chaining on the same enclave
        uint32_t returnValue = enclaveWasmModule->callFunction(argc, argv);
        if (returnValue != 0) {
            SPDLOG_ERROR_SGX("Error trying to call function. Return value: %i",
                             returnValue);
            return FAASM_SGX_WAMR_FUNCTION_UNABLE_TO_CALL;
        }

        return FAASM_SGX_SUCCESS;
    }

    faasm_sgx_status_t ecallCopyDataIn(uint8_t* buffer,
                                       uint32_t bufferSize,
                                       uint8_t* auxBuffer,
                                       uint32_t auxBufferSize)
    {
        auto* enclaveWasmModule = wasm::getExecutingEnclaveWasmModule();
        if (enclaveWasmModule == nullptr) {
            ocallLogError("Faaslet not bound to any module!");
            return FAASM_SGX_WAMR_MODULE_NOT_BOUND;
        }

        if (buffer != nullptr) {
            // Check if we need to malloc, or it has been malloc-ed for us
            // in advance
            if (enclaveWasmModule->dataXferPtr == nullptr) {
                enclaveWasmModule->dataXferPtr = (uint8_t*)malloc(bufferSize);
                enclaveWasmModule->dataXferSize = bufferSize;
            }

            assert(enclaveWasmModule->dataXferSize == bufferSize);
            memcpy(enclaveWasmModule->dataXferPtr, buffer, bufferSize);
        }

        if (auxBuffer != nullptr) {
            // Check if we need to malloc, or it has been malloc-ed for us
            // in advance
            if (enclaveWasmModule->dataXferAuxPtr == nullptr) {
                enclaveWasmModule->dataXferAuxPtr =
                  (uint8_t*)malloc(auxBufferSize);
                enclaveWasmModule->dataXferAuxSize = auxBufferSize;
            }

            assert(enclaveWasmModule->dataXferAuxSize == auxBufferSize);
            memcpy(enclaveWasmModule->dataXferAuxPtr, auxBuffer, auxBufferSize);
        }

        return FAASM_SGX_SUCCESS;
    }

    faasm_sgx_status_t ecallRunInternalTest(const char* testCase)
    {
        auto* enclaveWasmModule = wasm::getExecutingEnclaveWasmModule();
        if (enclaveWasmModule == nullptr) {
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
