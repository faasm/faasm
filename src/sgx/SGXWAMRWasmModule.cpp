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
    // Note that we set the second argument to true, so the string includes
    // the message id, making the key unique to the current executing task
    std::string funcStr = faabric::util::funcToString(msg, true);

    // Set up filesystem
    storage::FileSystem fs;
    fs.prepareFilesystem();

    // Load AoT
    storage::FileLoader& functionLoader = storage::getFileLoader();

    std::vector<uint8_t> wasmBytes =
      functionLoader.loadFunctionWamrAotFile(msg);
    assert(!wasmBytes.empty());

    // Load the wasm module
    wamrEnclave = acquireGlobalWAMREnclaveLock();

    wamrEnclave->loadWasmModule(funcStr, wasmBytes);

    // wamrEnclave = nullptr;
    releaseGlobalWAMREnclaveLock();

    SPDLOG_DEBUG("Binding SGX-WAMR module to function {}", funcStr);
    boundFuncStr = funcStr;

    // Set up the thread stacks
    // 28/06/2021 - Threading is not supported in SGX-WAMR. However, the Faasm
    // runtime expects (asserts) this vector to be non-empty. Change when
    // in-SGX threading is supported.
    threadStacks.push_back(-1);
}

void SGXWAMRWasmModule::reset(faabric::Message& msg,
                              const std::string& snapshotKey)
{
    // Note that we set the second argument to true, so the string includes
    // the message id, making the key unique to the current executing task
    std::string funcStr = faabric::util::funcToString(msg, true);

    wamrEnclave = acquireGlobalWAMREnclaveLock();
    wamrEnclave->unloadWasmModule(funcStr);
    releaseGlobalWAMREnclaveLock();

    boundFuncStr.clear();

    return;
}

int32_t SGXWAMRWasmModule::executeFunction(faabric::Message& msg)
{
    std::string funcStr = faabric::util::funcToString(msg, true);

    // Re-using WAMR modules across different tasks (running in the same
    // executor) is not supported. Thus, it may happen that we call this method
    // without having bound the module to the function (again).
    if (boundFuncStr != funcStr) {
        doBindToFunction(msg, false);
    }

    // Set execution context
    wasm::WasmExecutionContext ctx(this, &msg);

    // Note that we don't acquire a lock to enter the enclave to execute a
    // function. Multiple functions may run on the enclave at the same time
    // Call main function for module loaded into enclave
    wamrEnclave->callMainFunction(funcStr);

    // wamrEnclave = nullptr;
    // releaseGlobalWAMREnclave();

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
