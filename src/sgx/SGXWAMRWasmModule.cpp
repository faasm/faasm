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
static std::string getSgxFunctionKey(const faabric::Message& msg)
{
    return faabric::util::funcToString(msg, true);
}

void SGXWAMRWasmModule::doFunctionLoad(std::shared_ptr<WAMREnclave> wamrEnclave,
                                       const faabric::Message& msg)
{
    std::string funcStr = getSgxFunctionKey(msg);

    // Set up filesystem
    storage::FileSystem fs;
    fs.prepareFilesystem();

    // Load AoT
    storage::FileLoader& functionLoader = storage::getFileLoader();

    std::vector<uint8_t> wasmBytes =
      functionLoader.loadFunctionWamrAotFile(msg);
    if (wasmBytes.empty()) {
        SPDLOG_ERROR("SGX-WAMR machine code for function {}/{} not found",
                     msg.user(),
                     msg.function());
        SPDLOG_ERROR("Make sure the WASM code has been uploaded beforehand with"
                     "the right flags for SGX-WAMR code generation");
        throw std::runtime_error("SGX-WAMR machine code not found");
    }

    wamrEnclave->loadWasmModule(funcStr, wasmBytes);
}

void SGXWAMRWasmModule::doBindToFunction(faabric::Message& msg, bool cache)
{
    // Note that we defer the actual loading of the function to the enclave
    // to execute-time
    std::string funcStr = getSgxFunctionKey(msg);

    SPDLOG_DEBUG("Binding SGX-WAMR module to function {}", funcStr);
    boundFuncStr = funcStr;

    // Set up the thread stacks
    // 28/06/2021 - Threading is not supported in SGX-WAMR. However, the Faasm
    // runtime expects (asserts) this vector to be non-empty. Change when
    // in-SGX threading is supported.
    threadStacks.push_back(-1);
}

void SGXWAMRWasmModule::clearCaches()
{
    SPDLOG_DEBUG("Clearing SGX enclave and caches");

    std::shared_ptr<WAMREnclave> wamrEnclave = acquireGlobalWAMREnclaveLock();
    wamrEnclave->tearDownSgxEnclave();
    releaseGlobalWAMREnclaveLock();
}

void SGXWAMRWasmModule::reset(faabric::Message& msg,
                              const std::string& snapshotKey)
{
    if (!isBound()) {
        return;
    }

    std::string funcStr = getSgxFunctionKey(msg);
    assert(funcStr == boundFuncStr);
}

int32_t SGXWAMRWasmModule::executeFunction(faabric::Message& msg)
{
    std::string funcStr = getSgxFunctionKey(msg);

    SPDLOG_DEBUG("Executing SGX-WAMR module {}", funcStr);

    if (!isBound()) {
        SPDLOG_ERROR("SGX-WAMR module must be bound before executing task: {}",
                     funcStr);
        throw std::runtime_error("Module must be bound before executing");
    }

    if (boundFuncStr.empty()) {
        SPDLOG_ERROR("SGX-WAMR module is bound but bounded function is empty "
                     "(funcStr: {})",
                     funcStr);
        throw std::runtime_error("Module must be bound before executing");
    }

    // Re-using WAMR modules across different tasks (running in the same
    // executor) is not supported. Thus, it may happen that we call this method
    // without having bound the module to the function (again).
    if (boundFuncStr != funcStr) {
        SPDLOG_ERROR("Requested function to execute different to the one the"
                     "module is bound to (bound: {} != req: {})",
                     boundFuncStr,
                     funcStr);
        doBindToFunction(msg, false);
    }

    // Given that the enclave is shared across all Faaslets, function upload
    // a global lock as it modifies the in-enclave module cache
    wamrEnclave = acquireGlobalWAMREnclaveLock();

    // Load the WAMR machine code to the enclave
    doFunctionLoad(wamrEnclave, msg);

    releaseGlobalWAMREnclaveLock();

    // Set execution context
    wasm::WasmExecutionContext ctx(this, &msg);

    wamrEnclave->callMainFunction(funcStr);

    SPDLOG_DEBUG("Finished executing SGX-WAMR module {}", funcStr);

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
