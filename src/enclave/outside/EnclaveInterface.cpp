#include <enclave/outside/EnclaveInterface.h>
#include <enclave/outside/ecalls.h>
#include <enclave/outside/system.h>
#include <faabric/util/gids.h>
#include <wasm/WasmExecutionContext.h>

#include <faabric/util/func.h>

using namespace sgx;

namespace wasm {

EnclaveInterface::EnclaveInterface()
  : interfaceId(faabric::util::generateGid())
{
    checkSgxSetup();

    SPDLOG_DEBUG("Created enclave interface for enclave {}",
                 sgx::getGlobalEnclaveId());
}

EnclaveInterface::~EnclaveInterface()
{
    unbindFunction();
}

// ----- Module lifecycle -----
void EnclaveInterface::doBindToFunction(faabric::Message& msg, bool cache)
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
    sgx_status_t status = ecallLoadModule(sgx::getGlobalEnclaveId(),
                                          &returnValue,
                                          (void*)wasmBytes.data(),
                                          (uint32_t)wasmBytes.size(),
                                          interfaceId);
    processECallErrors("Unable to enter enclave", status, returnValue);

    // Set up the thread stacks
    // 28/06/2021 - Threading is not supported in SGX-WAMR. However, the Faasm
    // runtime expects (asserts) this vector to be non-empty. Change when
    // in-SGX threading is supported.
    threadStacks.push_back(-1);
}

bool EnclaveInterface::unbindFunction()
{
    if (!isBound()) {
        return true;
    }

    SPDLOG_DEBUG("Unloading SGX wasm module");

    faasm_sgx_status_t returnValue;
    sgx_status_t sgxReturnValue =
      ecallUnloadModule(sgx::getGlobalEnclaveId(), &returnValue, interfaceId);
    processECallErrors("Error trying to unload module from enclave",
                       sgxReturnValue,
                       returnValue);

    return true;
}

int32_t EnclaveInterface::executeFunction(faabric::Message& msg)
{

    std::string funcStr = faabric::util::funcToString(msg, true);

    SPDLOG_DEBUG(
      "Entering enclave {} to execute {}", sgx::getGlobalEnclaveId(), funcStr);

    // Set execution context
    wasm::WasmExecutionContext ctx(this);

    // Enter enclave and call function
    faasm_sgx_status_t returnValue;
    sgx_status_t sgxReturnValue =
      ecallCallFunction(sgx::getGlobalEnclaveId(), &returnValue, interfaceId);
    processECallErrors(
      "Error running function inside enclave", sgxReturnValue, returnValue);

    return 0;
}

uint32_t EnclaveInterface::growMemory(size_t nBytes)
{
    SPDLOG_DEBUG("SGX-WAMR growing memory by {}", nBytes);

    uint32_t memBase = currentBrk.load(std::memory_order_acquire);

    uint32_t nPages = getNumberOfWasmPagesForBytes(nBytes);
    SPDLOG_WARN("Growing memory in SGX-WAMR never allocates new memory");

    currentBrk.store(memBase + (nPages * WASM_BYTES_PER_PAGE),
                     std::memory_order_release);
    return memBase;
}

uint32_t EnclaveInterface::shrinkMemory(size_t nBytes)
{
    SPDLOG_WARN("SGX-WAMR ignoring shrink memory");
    return 0;
}

size_t EnclaveInterface::getMemorySizeBytes()
{
    SPDLOG_WARN("SGX-WAMR getMemorySizeBytes not implemented");
    return 0;
}

uint8_t* EnclaveInterface::getMemoryBase()
{
    SPDLOG_WARN("SGX-WAMR getMemoryBase not implemented");
    return nullptr;
}
}
