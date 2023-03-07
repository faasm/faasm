#include <enclave/outside/EnclaveInterface.h>
#include <enclave/outside/ecalls.h>
#include <enclave/outside/system.h>
#include <faabric/util/gids.h>
#include <faabric/util/logging.h>
#include <wasm/WasmExecutionContext.h>

#include <faabric/util/func.h>

using namespace sgx;

namespace wasm {

EnclaveInterface::EnclaveInterface()
  : interfaceId(faabric::util::generateGid())
{
    enclaveId = checkSgxSetup();

    SPDLOG_DEBUG("Created enclave interface for enclave {}", enclaveId);
}

EnclaveInterface::~EnclaveInterface()
{
    SPDLOG_TRACE(
      "Destructing SGX-WAMR wasm module {}/{}", boundUser, boundFunction);

    faasm_sgx_status_t returnValue;
    sgx_status_t sgxReturnValue =
      ecallDestroyModule(enclaveId, &returnValue, interfaceId);
    processECallErrors("Error trying to unload module from enclave",
                       sgxReturnValue,
                       returnValue);
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
    sgx_status_t status = ecallReset(enclaveId,
                                     &returnValue,
                                     interfaceId,
                                     msg.user().c_str(),
                                     msg.function().c_str());
    processECallErrors("Unable to enter enclave", status, returnValue);
}

void EnclaveInterface::doBindToFunction(faabric::Message& msg, bool cache)
{
    SPDLOG_INFO("SGX-WAMR binding to {}/{} via message {} (id: {})",
                msg.user(),
                msg.function(),
                msg.id(),
                interfaceId);

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
                                                interfaceId);
    processECallErrors("Unable to enter enclave", status, returnValue);

    // Set up the thread stacks
    // 28/06/2021 - Threading is not supported in SGX-WAMR. However, the Faasm
    // runtime expects (asserts) this vector to be non-empty. Change when
    // in-SGX threading is supported.
    threadStacks.push_back(-1);
}

/*
bool EnclaveInterface::unbindFunction()
{
    if (!isBound()) {
        return true;
    }

    SPDLOG_DEBUG("Unloading SGX wasm module");

    return true;
}
*/

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
    for (int i = 0; i < argv.size(); i++) {
        cArgv.at(i) = const_cast<char*>(argv.at(i).c_str());
    }

    // Enter enclave and call function
    faasm_sgx_status_t returnValue;
    sgx_status_t sgxReturnValue =
      ecallCallFunction(enclaveId, &returnValue, interfaceId, argc, &cArgv[0]);
    processECallErrors(
      "Error running function inside enclave", sgxReturnValue, returnValue);

    return 0;
}

size_t EnclaveInterface::getMemorySizeBytes()
{
    // This method is implemented in src/enclave/inside/EnclaveWasmModule.cpp
    return 0;
}

size_t EnclaveInterface::getMaxMemoryPages()
{
    // This method is implemented in src/enclave/inside/EnclaveWasmModule.cpp
    return 0;
}

uint8_t* EnclaveInterface::getMemoryBase()
{
    // This method is implemented in src/enclave/inside/EnclaveWasmModule.cpp
    return nullptr;
}
}
