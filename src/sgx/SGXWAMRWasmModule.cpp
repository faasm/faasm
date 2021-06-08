#include <cstdio>

#include <faabric/util/func.h>
#include <sgx/SGXWAMRWasmModule.h>
#include <sgx/faasm_sgx_attestation.h>
#include <sgx/faasm_sgx_system.h>
#include <wasm/WasmExecutionContext.h>

extern "C"
{
    void ocall_printf(const char* msg) { printf("%s", msg); }
}

__thread faaslet_sgx_msg_buffer_t* faasletSgxMsgBufferPtr;

using namespace sgx;

namespace wasm {
SGXWAMRWasmModule::SGXWAMRWasmModule()
{
    auto logger = faabric::util::getLogger();

    // Allocate memory for response
    sgxWamrMsgResponse.buffer_len =
      (sizeof(sgx_wamr_msg_t) + sizeof(sgx_wamr_msg_hdr_t));
    sgxWamrMsgResponse.buffer_ptr =
      (sgx_wamr_msg_t*)calloc(sgxWamrMsgResponse.buffer_len, sizeof(uint8_t));

    if (!sgxWamrMsgResponse.buffer_ptr) {
        logger->error(
          "Unable to allocate space for SGX message response buffer");
        throw std::runtime_error(
          "Unable to allocate space for SGX message response buffer");
    }

    faasletSgxMsgBufferPtr = &sgxWamrMsgResponse;

    logger->debug("Created SGX wasm module for enclave {}", globalEnclaveId);
}

SGXWAMRWasmModule::~SGXWAMRWasmModule()
{
    unbindFunction();

    if (sgxWamrMsgResponse.buffer_ptr) {
        free(sgxWamrMsgResponse.buffer_ptr);
    }
}

void SGXWAMRWasmModule::doBindToFunction(faabric::Message& msg, bool cache)
{
    auto logger = faabric::util::getLogger();

    // Set up filesystem
    storage::FileSystem fs;
    fs.prepareFilesystem();

    // Load AoT or wasm
    storage::FileLoader& fl = storage::getFileLoader();

#if (FAASM_SGX_WAMR_AOT_MODE)
    std::vector<uint8_t> wasmBytes = fl.loadFunctionWamrAotFile(msg);
#else
    std::vector<uint8_t> wasmBytes = fl.loadFunctionWasm(msg);
#endif

    // Load the wasm module
    faasm_sgx_status_t returnValue;
    sgx_status_t status =
      faasm_sgx_enclave_load_module(globalEnclaveId,
                                    &returnValue,
                                    (void*)wasmBytes.data(),
                                    (uint32_t)wasmBytes.size(),
                                    &threadId
#if (FAASM_SGX_ATTESTATION)
                                    ,
                                    &(faasletSgxMsgBufferPtr->buffer_ptr)
#endif
      );

    if (status != SGX_SUCCESS) {
        logger->error("Unable to enter enclave: {}", sgxErrorString(status));
        throw std::runtime_error("Unable to enter enclave");
    }

    if (returnValue != FAASM_SGX_SUCCESS) {
        logger->error("Unable to load WASM module: {}",
                      faasmSgxErrorString(returnValue));
        throw std::runtime_error("Unable to load WASM module");
    }
}

bool SGXWAMRWasmModule::unbindFunction()
{
    if (!isBound()) {
        return true;
    }

    auto logger = faabric::util::getLogger();
    logger->debug("Unloading SGX wasm module");

    faasm_sgx_status_t returnValue;
    sgx_status_t sgxReturnValue =
      faasm_sgx_enclave_unload_module(globalEnclaveId, &returnValue, threadId);

    if (sgxReturnValue != SGX_SUCCESS) {
        logger->error("Unable to unbind function due to SGX error: {}",
                      sgxErrorString(sgxReturnValue));
        throw std::runtime_error("Unable to unbind function due to SGX error");
    }

    if (returnValue != FAASM_SGX_SUCCESS) {
        logger->error("Unable to unbind function: {}",
                      faasmSgxErrorString(returnValue));
        throw std::runtime_error("Unable to unbind function");
    }

    return true;
}

int32_t SGXWAMRWasmModule::executeFunction(faabric::Message& msg)
{
    auto logger = faabric::util::getLogger();
    std::string funcStr = faabric::util::funcToString(msg, true);

    logger->debug(
      "Entering enclave {} to execute {}", globalEnclaveId, funcStr);

    // Set execution context
    wasm::WasmExecutionContext ctx(this, &msg);

    // Enter enclave and call function
    faasm_sgx_status_t returnValue;
    sgx_status_t sgxReturnValue = faasm_sgx_enclave_call_function(
      globalEnclaveId, &returnValue, threadId, msg.funcptr());

    if (sgxReturnValue != SGX_SUCCESS) {
        logger->error("Unable to enter enclave: {}",
                      sgxErrorString(sgxReturnValue));
        throw std::runtime_error("Unable to enter enclave");
    }

    if (returnValue != FAASM_SGX_SUCCESS) {
        // Check if an ocall has failed
        sgxReturnValue =
          (sgx_status_t)FAASM_SGX_OCALL_GET_SGX_ERROR(returnValue);
        if (sgxReturnValue) {
            logger->error("An OCALL failed: {}",
                          sgxErrorString(sgxReturnValue));
            throw std::runtime_error("OCALL failed");
        }

        logger->error("Error occurred during function execution: {}",
                      faasmSgxErrorString(returnValue));
        throw std::runtime_error("Error occurred during function execution");
    }

    return 0;
}
}
