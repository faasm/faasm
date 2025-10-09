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
    enclaveId = createEnclave();

    SPDLOG_DEBUG("Created enclave interface for enclave {}", enclaveId);
}

EnclaveInterface::~EnclaveInterface()
{
    SPDLOG_TRACE(
      "Destructing SGX-WAMR wasm module {}/{}", boundUser, boundFunction);

    // Tear-down WASM module inside enclave
    faasm_sgx_status_t returnValue;
    sgx_status_t sgxReturnValue =
      ecallDestroyModule(enclaveId, &returnValue, interfaceId);
    processECallErrors("Error trying to unload module from enclave",
                       sgxReturnValue,
                       returnValue);

    // Destroy the enclave
    destroyEnclave(enclaveId);
    enclaveId = 0;
}

EnclaveInterface* getExecutingEnclaveInterface()
{
    return reinterpret_cast<EnclaveInterface*>(getExecutingModule());
}

// ----- Module lifecycle -----

void EnclaveInterface::reset(faabric::Message& msg,
                             const std::string& snapshotKey)
{
    if (!_isBound) {
        return;
    }

    faasm_sgx_status_t returnValue;
    sgx_status_t status = ecallReset(enclaveId, &returnValue);
    processECallErrors("Unable to enter enclave", status, returnValue);
}

void EnclaveInterface::doBindToFunction(faabric::Message& msg, bool cache)
{
    // Work-out whether to  use Accless or not
    bool enableAccless = conf::getFaasmConfig().acclessEnabled == "on";

    SPDLOG_INFO(
      "SGX-WAMR binding to {}/{} via message {} (eid: {} - Accless: {})",
      msg.user(),
      msg.function(),
      msg.id(),
      enclaveId,
      enableAccless ? "on" : "off");

    // Set up filesystem
    filesystem.prepareFilesystem();

    // Load AoT
    storage::FileLoader& functionLoader = storage::getFileLoader();

    std::vector<uint8_t> wasmBytes =
      functionLoader.loadFunctionWamrAotFile(msg);

    // Bind the enclave wasm module to the function
    faasm_sgx_status_t returnValue;
    sgx_status_t status = ecallDoBindToFunction(enclaveId,
                                                &returnValue,
                                                msg.user().c_str(),
                                                msg.function().c_str(),
                                                (void*)wasmBytes.data(),
                                                (uint32_t)wasmBytes.size(),
                                                enableAccless);
    processECallErrors("Unable to enter enclave", status, returnValue);

    // Set up the thread stacks
    // 28/06/2021 - Threading is not supported in SGX-WAMR. However, the Faasm
    // runtime expects (asserts) this vector to be non-empty. Change when
    // in-SGX threading is supported.
    threadStacks.push_back(-1);
}

int32_t EnclaveInterface::executeFunction(faabric::Message& msg)
{
    std::string funcStr = faabric::util::funcToString(msg, true);

    SPDLOG_DEBUG("Entering enclave {} to execute {}", enclaveId, funcStr);

    // Set execution context
    wasm::WasmExecutionContext ctx(this);

    // Prepare argc/argv to be passed to the enclave.
    std::vector<std::string> argv = faabric::util::getArgvForMessage(msg);
    int argc = argv.size();
    // Convert to vector of char pointers for easier serialisation
    std::vector<char*> cArgv;
    cArgv.resize(argv.size());
    for (size_t i = 0; i < argv.size(); i++) {
        cArgv.at(i) = const_cast<char*>(argv.at(i).c_str());
    }

    // Enter enclave and call function
    faasm_sgx_status_t returnValue;
    sgx_status_t sgxReturnValue = ecallCallFunction(
      enclaveId, &returnValue, interfaceId, argc, cArgv.data());
    processECallErrors(
      "Error running function inside enclave", sgxReturnValue, returnValue);

    return 0;
}

size_t EnclaveInterface::getMemorySizeBytes()
{
    SPDLOG_WARN("SGX-WAMR getMemorySizeBytes not implemented");
    return 0;
}

size_t EnclaveInterface::getMaxMemoryPages()
{
    SPDLOG_WARN("SGX-WAMR getMaxMemoryPages not implemented");
    return 0;
}

uint8_t* EnclaveInterface::getMemoryBase()
{
    SPDLOG_WARN("SGX-WAMR getMemoryBase not implemented");
    return nullptr;
}
}
