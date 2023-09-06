#include <conf/FaasmConfig.h>
#ifndef FAASM_SGX_DISABLED_MODE
#include <enclave/outside/EnclaveInterface.h>
#include <enclave/outside/system.h>
#endif
#include <faabric/scheduler/Scheduler.h>
#include <faabric/snapshot/SnapshotRegistry.h>
#include <faabric/util/batch.h>
#include <faabric/util/config.h>
#include <faabric/util/environment.h>
#include <faabric/util/gids.h>
#include <faabric/util/locks.h>
#include <faabric/util/logging.h>
#include <faabric/util/timing.h>
#include <faaslet/Faaslet.h>
#include <storage/FileLoader.h>
#include <storage/FileSystem.h>
#include <system/CGroup.h>
#include <system/NetworkNamespace.h>
#include <threads/ThreadState.h>
#include <wamr/WAMRWasmModule.h>
#include <wavm/WAVMWasmModule.h>

#include <stdexcept>

static thread_local bool threadIsIsolated = false;

using namespace isolation;

namespace faaslet {

void preloadPythonRuntime()
{
    conf::FaasmConfig& conf = conf::getFaasmConfig();
    if (conf.pythonPreload != "on") {
        SPDLOG_INFO("Not preloading python runtime");
        return;
    }

    SPDLOG_INFO("Preparing python runtime");

    auto req = faabric::util::batchExecFactory(PYTHON_USER, PYTHON_FUNC, 1);
    auto& msg = *req->mutable_messages(0);
    msg.set_ispython(true);
    msg.set_pythonuser("python");
    msg.set_pythonfunction("noop");

    auto& plannerCli = faabric::planner::getPlannerClient();
    plannerCli.callFunctions(req);
}

Faaslet::Faaslet(faabric::Message& msg)
  : Executor(msg)
{
    conf::FaasmConfig& conf = conf::getFaasmConfig();

    // Instantiate the right wasm module for the chosen runtime
    if (conf.wasmVm == "sgx") {
#ifndef FAASM_SGX_DISABLED_MODE
        module = std::make_unique<wasm::EnclaveInterface>();
#else
        SPDLOG_ERROR(
          "SGX WASM VM selected, but SGX support disabled in config");
        throw std::runtime_error("SGX support disabled in config");
#endif
    } else if (conf.wasmVm == "wamr") {
        // Vanilla WAMR
        module = std::make_unique<wasm::WAMRWasmModule>(threadPoolSize);
    } else if (conf.wasmVm == "wavm") {
        module = std::make_unique<wasm::WAVMWasmModule>(threadPoolSize);
    } else {

        SPDLOG_ERROR("Unrecognised wasm VM: {}", conf.wasmVm);
        throw std::runtime_error("Unrecognised wasm VM");
    }

    // Bind to the function
    module->bindToFunction(msg);

    // Create the reset snapshot for this function if it doesn't already exist
    // (currently only supported in WAVM)
    if (conf.wasmVm == "wavm") {
        localResetSnapshotKey =
          wasm::getWAVMModuleCache().registerResetSnapshot(*module, msg);
    }
}

int32_t Faaslet::executeTask(int threadPoolIdx,
                             int msgIdx,
                             std::shared_ptr<faabric::BatchExecuteRequest> req)
{
    // Lazily setup Faaslet isolation.
    // This has to be done within the same thread as the execution (hence we
    // leave it until just before execution).
    // Because this is a thread-specific operation we don't need any
    // synchronisation here, and rely on the cgroup and network namespace
    // operations being thread-safe.

    if (!threadIsIsolated) {
        // Add this thread to the cgroup
        CGroup cgroup(BASE_CGROUP_NAME);
        cgroup.addCurrentThread();

        // Set up network namespace
        ns = claimNetworkNamespace();
        ns->addCurrentThread();

        threadIsIsolated = true;
    }

    int32_t returnValue = module->executeTask(threadPoolIdx, msgIdx, req);

    return returnValue;
}

void Faaslet::reset(faabric::Message& msg)
{
    faabric::scheduler::Executor::reset(msg);
    module->reset(msg, localResetSnapshotKey);
}

void Faaslet::shutdown()
{
    if (ns != nullptr) {
        ns->removeCurrentThread();
        returnNetworkNamespace(ns);
    }

    Executor::shutdown();
}

std::span<uint8_t> Faaslet::getMemoryView()
{
    return module->getMemoryView();
}

void Faaslet::setMemorySize(size_t newSize)
{
    module->setMemorySize(newSize);
}

size_t Faaslet::getMaxMemorySize()
{
    return MAX_WASM_MEM;
}

std::string Faaslet::getLocalResetSnapshotKey()
{
    return localResetSnapshotKey;
}

FaasletFactory::~FaasletFactory() {}

std::shared_ptr<faabric::scheduler::Executor> FaasletFactory::createExecutor(
  faabric::Message& msg)
{
    return std::make_shared<Faaslet>(msg);
}

void FaasletFactory::flushHost()
{
    // Clear cached wasm and object files
    storage::FileLoader& fileLoader = storage::getFileLoader();
    fileLoader.clearLocalCache();

    // WAVM-specific flushing
    const conf::FaasmConfig& conf = conf::getFaasmConfig();
    if (conf.wasmVm == "wavm") {
        wasm::WAVMWasmModule::clearCaches();
    }
}
}
