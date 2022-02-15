#include <cstdio>

#include <enclave/outside/SGXWAMRWasmModule.h>
#include <enclave/outside/system.h>
#include <faabric/util/func.h>
#include <wasm/WasmExecutionContext.h>

extern "C"
{
    void ocall_printf(const char* msg) { printf("%s", msg); }
}

using namespace sgx;

namespace wasm {
SGXWAMRWasmModule::SGXWAMRWasmModule()
{
    checkSgxSetup();

    SPDLOG_DEBUG("Created SGX wasm module for enclave {}",
                 sgx::getGlobalEnclaveId());
}

SGXWAMRWasmModule::~SGXWAMRWasmModule()
{
    unbindFunction();
}

// ----- Module lifecycle -----
void SGXWAMRWasmModule::doBindToFunction(faabric::Message& msg, bool cache)
{
    // Set up filesystem
    storage::FileSystem fs;
    fs.prepareFilesystem();

    // Load AoT
    storage::FileLoader& functionLoader = storage::getFileLoader();

    std::vector<uint8_t> wasmBytes =
      functionLoader.loadFunctionWamrAotFile(msg);

    // Load the wasm module
    // Note - loading and instantiating happen in the same ecall
    faasm_sgx_status_t returnValue;
    sgx_status_t status =
      faasm_sgx_enclave_load_module(sgx::getGlobalEnclaveId(),
                                    &returnValue,
                                    (void*)wasmBytes.data(),
                                    (uint32_t)wasmBytes.size(),
                                    &threadId);

    if (status != SGX_SUCCESS) {
        SPDLOG_ERROR("Unable to enter enclave: {}", sgxErrorString(status));
        throw std::runtime_error("Unable to enter enclave");
    }

    if (returnValue != FAASM_SGX_SUCCESS) {
        SPDLOG_ERROR("Unable to load WASM module: {}",
                     faasmSgxErrorString(returnValue));
        throw std::runtime_error("Unable to load WASM module");
    }

    // Set up the thread stacks
    // 28/06/2021 - Threading is not supported in SGX-WAMR. However, the Faasm
    // runtime expects (asserts) this vector to be non-empty. Change when
    // in-SGX threading is supported.
    threadStacks.push_back(-1);
}

bool SGXWAMRWasmModule::unbindFunction()
{
    if (!isBound()) {
        return true;
    }

    SPDLOG_DEBUG("Unloading SGX wasm module");

    faasm_sgx_status_t returnValue;
    sgx_status_t sgxReturnValue = faasm_sgx_enclave_unload_module(
      sgx::getGlobalEnclaveId(), &returnValue, threadId);

    if (sgxReturnValue != SGX_SUCCESS) {
        SPDLOG_ERROR("Unable to unbind function due to SGX error: {}",
                     sgxErrorString(sgxReturnValue));
        throw std::runtime_error("Unable to unbind function due to SGX error");
    }

    if (returnValue != FAASM_SGX_SUCCESS) {
        SPDLOG_ERROR("Unable to unbind function: {}",
                     faasmSgxErrorString(returnValue));
        throw std::runtime_error("Unable to unbind function");
    }

    return true;
}

int32_t SGXWAMRWasmModule::executeFunction(faabric::Message& msg)
{

    std::string funcStr = faabric::util::funcToString(msg, true);

    SPDLOG_DEBUG(
      "Entering enclave {} to execute {}", sgx::getGlobalEnclaveId(), funcStr);

    // Set execution context
    wasm::WasmExecutionContext ctx(this, &msg);

    // Enter enclave and call function
    faasm_sgx_status_t returnValue;
    sgx_status_t sgxReturnValue = faasm_sgx_enclave_call_function(
      sgx::getGlobalEnclaveId(), &returnValue, threadId);

    if (sgxReturnValue != SGX_SUCCESS) {
        SPDLOG_ERROR("Unable to enter enclave: {}",
                     sgxErrorString(sgxReturnValue));
        throw std::runtime_error("Unable to enter enclave");
    }

    if (returnValue != FAASM_SGX_SUCCESS) {
        // Check if an ocall has failed
        sgxReturnValue =
          (sgx_status_t)FAASM_SGX_OCALL_GET_SGX_ERROR(returnValue);
        if (sgxReturnValue) {
            SPDLOG_ERROR("An OCALL failed: {}", sgxErrorString(sgxReturnValue));
            throw std::runtime_error("OCALL failed");
        }

        SPDLOG_ERROR("Error occurred during function execution: {}",
                     faasmSgxErrorString(returnValue));
        throw std::runtime_error("Error occurred during function execution");
    }

    return 0;
}

uint32_t SGXWAMRWasmModule::growMemory(size_t nBytes)
{
    SPDLOG_DEBUG("SGX-WAMR growing memory by {}", nBytes);

    uint32_t memBase = currentBrk.load(std::memory_order_acquire);

    uint32_t nPages = getNumberOfWasmPagesForBytes(nBytes);
    SPDLOG_WARN("Growing memory in SGX-WAMR never allocates new memory");

    currentBrk.store(memBase + (nPages * WASM_BYTES_PER_PAGE),
                     std::memory_order_release);
    return memBase;
}

uint32_t SGXWAMRWasmModule::shrinkMemory(size_t nBytes)
{
    SPDLOG_WARN("SGX-WAMR ignoring shrink memory");
    return 0;
}

size_t SGXWAMRWasmModule::getMemorySizeBytes()
{
    SPDLOG_WARN("SGX-WAMR getMemorySizeBytes not implemented");
    return 0;
}

uint8_t* SGXWAMRWasmModule::getMemoryBase()
{
    SPDLOG_WARN("SGX-WAMR getMemoryBase not implemented");
    return nullptr;
}
}
