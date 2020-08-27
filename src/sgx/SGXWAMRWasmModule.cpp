#include <cstdio>

#include <sgx/SGXWAMRWasmModule.h>
#include <sgx/sgx_wamr_attestation.h>

extern "C" {
void ocall_printf(const char *msg) {
    printf("%s", msg);
}
}

__thread faaslet_sgx_msg_buffer_t *faaslet_sgx_msg_buffer_ptr;

namespace wasm {
    SGXWAMRWasmModule::SGXWAMRWasmModule(sgx_enclave_id_t *enclaveId) : enclaveIdPtr(enclaveId) {
        sgx_wamr_msg_response.buffer_len = (sizeof(sgx_wamr_msg_t) + sizeof(sgx_wamr_msg_hdr_t));

        if (!(sgx_wamr_msg_response.buffer_ptr = (sgx_wamr_msg_t *) calloc(sgx_wamr_msg_response.buffer_len,
                                                                           sizeof(uint8_t)))) {
            printf("[Error] Unable to allocate space for faaslet message response buffer\n");
            abort();
        }

        faaslet_sgx_msg_buffer_ptr = &sgx_wamr_msg_response;
    }

    SGXWAMRWasmModule::~SGXWAMRWasmModule() {
        if (!unbindFunction()) {
            printf("[Error] SGX Faaslet destruction failed\n");
        }

        if (sgx_wamr_msg_response.buffer_ptr) {
            free(sgx_wamr_msg_response.buffer_ptr);
        }
    }

    void SGXWAMRWasmModule::bindToFunction(const faabric::Message &msg) {
        // Load the function
        storage::FileSystem fs;
        storage::FileLoader &fl = storage::getFileLoader();
        fs.prepareFilesystem();
        wasmOpcode = fl.loadFunctionWasm(msg);

        // Set up the enclave
        faasm_sgx_status_t returnValue;
        sgx_status_t status = sgx_wamr_enclave_load_module(
                *enclaveIdPtr,
                &returnValue,
                (void *) wasmOpcode.data(),
                (uint32_t) wasmOpcode.size(),
                &threadId
#if(FAASM_SGX_ATTESTATION)
                , &faaslet_sgx_msg_buffer_ptr->buffer_ptr
#endif
        );

        if (status != SGX_SUCCESS) {
            printf("[Error] Unable to enter enclave (%#010x)\n", status);
            throw std::runtime_error("Unable to enter enclave");
        }

        if (returnValue != FAASM_SGX_SUCCESS) {
            printf("[Error] Unable to load WASM module (%#010x)\n", returnValue);
            throw std::runtime_error("Unable to load SGX WASM module");
        }

        _isBound = true;
    }

    void SGXWAMRWasmModule::bindToFunctionNoZygote(const faabric::Message &msg) {
        bindToFunction(msg);
    }

    bool SGXWAMRWasmModule::unbindFunction() {
        if (!_isBound) {
            return true;
        }

        faasm_sgx_status_t returnValue;
        sgx_status_t sgxReturnValue = sgx_wamr_enclave_unload_module(
                *enclaveIdPtr, &returnValue, threadId
        );

        if (sgxReturnValue != SGX_SUCCESS) {
            printf("[Error] Unable to enter enclave (%#010x)\n", sgxReturnValue);
            return false;
        }

        if (returnValue != FAASM_SGX_SUCCESS) {
            printf("[Error] Unable to unbind function (%#010x)\n", returnValue);
            return false;
        }

        return true;
    }

    bool SGXWAMRWasmModule::execute(faabric::Message &msg, bool forceNoop) {
        if (!_isBound) {
            printf("[Error] Unable to call desired function (%#010x)\n", FAASM_SGX_WAMR_MODULE_NOT_BOUND);
            msg.set_returnvalue(FAASM_SGX_WAMR_MODULE_NOT_BOUND);
            return false;
        }

        // Set executing call
        wasm::setExecutingCall(const_cast<faabric::Message *>(&msg));

        // Set up enclave
        faasm_sgx_status_t returnValue;
        sgx_status_t sgxReturnValue = sgx_wamr_enclave_call_function(
                *enclaveIdPtr, &returnValue, threadId, msg.idx()
        );

        if (sgxReturnValue != SGX_SUCCESS) {
            printf("[Error] Unable to enter enclave (%#010x)\n", sgxReturnValue);
            msg.set_returnvalue(FAASM_SGX_UNABLE_TO_ENTER_ENCLAVE);
            return false;
        }

        if (returnValue != FAASM_SGX_SUCCESS) {
            // Check if an ocall has failed
            sgxReturnValue = (sgx_status_t) FAASM_SGX_OCALL_GET_SGX_ERROR(returnValue);
            if (sgxReturnValue) {
                printf("[Error] An OCALL failed (%#010x)\n", sgxReturnValue);
                msg.set_returnvalue(FAASM_SGX_OCALL_FAILED);
                return false;
            }

            printf("[Error] An error occurred during function execution (%#010x)\n", returnValue);
            msg.set_returnvalue(returnValue);

            return false;
        }

        return true;
    }

    bool SGXWAMRWasmModule::isBound() {
        return _isBound;
    }
}
