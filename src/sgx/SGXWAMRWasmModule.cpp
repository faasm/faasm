#include <cstdio>

#include <faabric/util/func.h>
#include <sgx/SGXWAMRWasmModule.h>
#include <sgx/sgx_wamr_attestation.h>
#include <sgx/sgx_system.h>

extern "C" {
void ocall_printf(const char *msg) {
    printf("%s", msg);
}
}

__thread faaslet_sgx_msg_buffer_t *faaslet_sgx_msg_buffer_ptr;

using namespace sgx;

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
        auto logger = faabric::util::getLogger();

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
            logger->error("Unable to enter enclave: {}", sgxErrorString(status));
            throw std::runtime_error("Unable to enter enclave");
        }

        if (returnValue != FAASM_SGX_SUCCESS) {
            logger->error("Unable to load WASM module: {}", faasmSgxErrorString(returnValue));
            throw std::runtime_error("Unable to load WASM module");
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

        auto logger = faabric::util::getLogger();

        faasm_sgx_status_t returnValue;
        sgx_status_t sgxReturnValue = sgx_wamr_enclave_unload_module(
                *enclaveIdPtr, &returnValue, threadId
        );

        if (sgxReturnValue != SGX_SUCCESS) {
            logger->error("Unable to enter enclave on bind: {}", sgxErrorString(sgxReturnValue));
            throw std::runtime_error("Unable to enter enclave on bind");
        }

        if (returnValue != FAASM_SGX_SUCCESS) {
            logger->error("Unable to unbind function: {}", faasmSgxErrorString(returnValue));
            throw std::runtime_error("Unable to unbind function");
        }

        return true;
    }

    bool SGXWAMRWasmModule::execute(faabric::Message &msg, bool forceNoop) {
        auto logger = faabric::util::getLogger();
        if (!_isBound) {
            logger->error("Function already bound {}", faabric::util::funcToString(msg, true));
            throw std::runtime_error("Function already bound");
        }

        // Set executing call
        wasm::setExecutingCall(const_cast<faabric::Message *>(&msg));

        // Set up enclave
        faasm_sgx_status_t returnValue;
        sgx_status_t sgxReturnValue = sgx_wamr_enclave_call_function(
                *enclaveIdPtr, &returnValue, threadId, msg.idx()
        );

        if (sgxReturnValue != SGX_SUCCESS) {
            logger->error("Unable to enter enclave: {}", sgxErrorString(sgxReturnValue));
            throw std::runtime_error("Unable to enter enclave");
        }

        if (returnValue != FAASM_SGX_SUCCESS) {
            // Check if an ocall has failed
            sgxReturnValue = (sgx_status_t) FAASM_SGX_OCALL_GET_SGX_ERROR(returnValue);
            if (sgxReturnValue) {
                logger->error("An OCALL failed: {}", sgxErrorString(sgxReturnValue));
                throw std::runtime_error("OCALL failed");
            }

            logger->error("Error occurred during function execution: {}", faasmSgxErrorString(returnValue));
            throw std::runtime_error("Error during function execution");
        }

        return true;
    }

    bool SGXWAMRWasmModule::isBound() {
        return _isBound;
    }
}
