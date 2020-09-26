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

__thread faaslet_sgx_msg_buffer_t *faasletSgxMsgBufferPtr;

using namespace sgx;

namespace wasm {
    SGXWAMRWasmModule::SGXWAMRWasmModule(){
        auto logger = faabric::util::getLogger();

        // Allocate memory for response //TODO
        sgxWamrMsgResponse.buffer_len = (sizeof(sgx_wamr_msg_t) + sizeof(sgx_wamr_msg_hdr_t));
        sgxWamrMsgResponse.buffer_ptr = (sgx_wamr_msg_t *) calloc(sgxWamrMsgResponse.buffer_len, sizeof(uint8_t));
        if (!sgxWamrMsgResponse.buffer_ptr) {
            logger->error("Unable to allocate space for SGX message response buffer");
            throw std::runtime_error("Unable to allocate space for SGX message response buffer");
        }
        faasletSgxMsgBufferPtr = &sgxWamrMsgResponse;

        logger->debug("Created SGX wasm module for enclave {}", globalEnclaveId);
    }

    SGXWAMRWasmModule::~SGXWAMRWasmModule() {
        unbindFunction();

        if (sgxWamrMsgResponse.buffer_ptr) {
            free(sgxWamrMsgResponse.buffer_ptr);
        }
    }

    void SGXWAMRWasmModule::bindToFunction(const faabric::Message &msg) {
        auto logger = faabric::util::getLogger();

        // Set up filesystem
        storage::FileSystem fs;
        fs.prepareFilesystem();

        // Load wasm (running WAMR in interpreter mode)
        storage::FileLoader &fl = storage::getFileLoader();
        std::vector<uint8_t> wasmBytes = fl.loadFunctionWasm(msg);

        // Load the wasm module
        faasm_sgx_status_t returnValue;
        sgx_status_t status = faasm_sgx_enclave_load_module(
                globalEnclaveId,
                &returnValue,
                (void *) wasmBytes.data(),
                (uint32_t) wasmBytes.size(),
                &threadId
#if(FAASM_SGX_ATTESTATION)
                , &(faasletSgxMsgBufferPtr->buffer_ptr)
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
        logger->debug("Unloading SGX wasm module");

        faasm_sgx_status_t returnValue;
        sgx_status_t sgxReturnValue = faasm_sgx_enclave_unload_module(
                globalEnclaveId, &returnValue, threadId
        );

        if (sgxReturnValue != SGX_SUCCESS) {
            logger->error("Unable to unbind function due to SGX error: {}", sgxErrorString(sgxReturnValue));
            throw std::runtime_error("Unable to unbind function due to SGX error");
        }

        if (returnValue != FAASM_SGX_SUCCESS) {
            logger->error("Unable to unbind function: {}", faasmSgxErrorString(returnValue));
            throw std::runtime_error("Unable to unbind function");
        }

        return true;
    }

    bool SGXWAMRWasmModule::execute(faabric::Message &msg, bool forceNoop) {
        auto logger = faabric::util::getLogger();
        std::string funcStr = faabric::util::funcToString(msg, true);
        if (!_isBound) {
            logger->error("Function not bound {}", funcStr);
            throw std::runtime_error("Function not bound");
        }

        // Set executing call
        wasm::setExecutingCall(const_cast<faabric::Message *>(&msg));

        // Enter enclave and call function
        faasm_sgx_status_t returnValue;
        logger->debug("Entering enclave {} to execute {}", globalEnclaveId, funcStr);
        sgx_status_t sgxReturnValue = faasm_sgx_enclave_call_function(
                globalEnclaveId, &returnValue, threadId, msg.idx()
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
            throw std::runtime_error("Error occurred during function execution");
        }

        return true;
    }

    bool SGXWAMRWasmModule::isBound() {
        return _isBound;
    }
}
