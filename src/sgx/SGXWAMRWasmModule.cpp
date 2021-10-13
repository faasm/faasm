#include <faabric/util/func.h>
#include <faabric/util/locks.h>
#include <sgx/SGXWAMRWasmModule.h>
#include <sgx/attestation.h>
#include <sgx/system.h>
#include <wasm/WasmExecutionContext.h>

#include <cstdio>

using namespace sgx;

namespace wasm {
std::mutex enclaveMx;

SGXWAMRWasmModule::SGXWAMRWasmModule()
  : wamrEnclave(sgx::getWamrEnclave())
{
    SPDLOG_DEBUG("Created SGX WAMR-Wasm module for enclave {}",
                 wamrEnclave.getId());
}

SGXWAMRWasmModule::~SGXWAMRWasmModule() {}

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
    if (!wamrEnclave.isWasmLoaded(wasmBytes)) {
        faabric::util::UniqueLock lock(enclaveMx);

        if (!wamrEnclave.isWasmLoaded(wasmBytes)) {
            wamrEnclave.loadWasmModule(wasmBytes);
        } else {
            SPDLOG_DEBUG("Module already loaded to enclave, skipping load");
        }
    } else {
        SPDLOG_DEBUG("Module already loaded to enclave, skipping load");
    }

    // Set up the thread stacks
    // 28/06/2021 - Threading is not supported in SGX-WAMR. However, the Faasm
    // runtime expects (asserts) this vector to be non-empty. Change when
    // in-SGX threading is supported.
    threadStacks.push_back(-1);
}

// TODO - reset from a snapshot key
void SGXWAMRWasmModule::reset(faabric::Message& msg,
                              const std::string& snapshotKey)
{
    if (!wamrEnclave.isWasmLoaded()) {
        SPDLOG_DEBUG("Module already unloaded from enclave, skipping unbind");
        return;
    }

    faabric::util::UniqueLock lock(enclaveMx);

    if (!wamrEnclave.isWasmLoaded()) {
        SPDLOG_DEBUG("Module already unloaded from enclave, skipping unbind");
        return;
    }

    wamrEnclave.unloadWasmModule();

    return;
}

int32_t SGXWAMRWasmModule::executeFunction(faabric::Message& msg)
{

    std::string funcStr = faabric::util::funcToString(msg, true);

    SPDLOG_DEBUG(
      "Entering enclave {} to execute {}", wamrEnclave.getId(), funcStr);

    // Set execution context
    wasm::WasmExecutionContext ctx(this, &msg);

    // Call main function for module loaded into enclave
    wamrEnclave.callMainFunction();

    return 0;
}

uint32_t SGXWAMRWasmModule::growMemory(uint32_t nBytes)
{
    SPDLOG_DEBUG("SGX-WAMR growing memory by {}", nBytes);

    uint32_t memBase = currentBrk;

    uint32_t nPages = getNumberOfWasmPagesForBytes(nBytes);
    SPDLOG_WARN("Growing memory in SGX-WAMR never allocates new memory");

    currentBrk = memBase + (nPages * WASM_BYTES_PER_PAGE);
    return memBase;
}

uint32_t SGXWAMRWasmModule::shrinkMemory(uint32_t nBytes)
{
    SPDLOG_WARN("SGX-WAMR ignoring shrink memory");
    return 0;
}
}
