#include <faabric/util/func.h>
#include <sgx/SGXWAMRWasmModule.h>
#include <sgx/attestation.h>
#include <sgx/system.h>
#include <wasm/WasmExecutionContext.h>

#include <cstdio>

using namespace sgx;

namespace wasm {
SGXWAMRWasmModule::SGXWAMRWasmModule()
{
    SPDLOG_DEBUG("Created SGX WAMR-Wasm module");
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
    assert(!wasmBytes.empty());

    // Load the wasm module
    wamrEnclave = acquireGlobalWAMREnclave();

    wamrEnclave->loadWasmModule(wasmBytes, &enclaveModuleSlot);
    SPDLOG_DEBUG("Loaded module to enclave's store at slot {}",
                 enclaveModuleSlot);

    wamrEnclave = nullptr;
    releaseGlobalWAMREnclave();

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
    wamrEnclave = acquireGlobalWAMREnclave();

    if (wamrEnclave->isSlotLoaded(enclaveModuleSlot)) {
        SPDLOG_DEBUG("Removing module (slot: {}) from the enclave (id: {}) store",
                     enclaveModuleSlot,
                     wamrEnclave->getId());
        wamrEnclave->unloadWasmModule(enclaveModuleSlot);
    } else {
        SPDLOG_DEBUG("Skipping removing module (slot: {}) from enclave (id: {})",
                     enclaveModuleSlot,
                     wamrEnclave->getId());
    }
    assert(!wamrEnclave->isSlotLoaded(enclaveModuleSlot));

    wamrEnclave = nullptr;
    releaseGlobalWAMREnclave();

    return;
}

int32_t SGXWAMRWasmModule::executeFunction(faabric::Message& msg)
{

    std::string funcStr = faabric::util::funcToString(msg, true);

    // Set execution context
    wasm::WasmExecutionContext ctx(this, &msg);

    wamrEnclave = acquireGlobalWAMREnclave();

    int returnValue;
    // Make sure our module slot has been loaded before
    if (wamrEnclave->isSlotLoaded(enclaveModuleSlot)) {
        SPDLOG_DEBUG(
          "Entering enclave {} to execute {}", wamrEnclave->getId(), funcStr);

        // Call main function for module loaded into enclave
        wamrEnclave->callMainFunction(enclaveModuleSlot);

        returnValue = 0;
    } else {
        SPDLOG_ERROR("Enclave {} does not have function {} loaded in slot {}",
                     wamrEnclave->getId(),
                     funcStr,
                     enclaveModuleSlot);

        returnValue = 1;
    }

    wamrEnclave = nullptr;
    releaseGlobalWAMREnclave();

    return returnValue;
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
