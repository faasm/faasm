#include <faaslet/Faaslet.h>

#include <system/CGroup.h>
#include <system/NetworkNamespace.h>
#include <threads/ThreadState.h>
#include <wamr/WAMRWasmModule.h>
#include <wavm/WAVMWasmModule.h>

#include <faabric/scheduler/Scheduler.h>
#include <faabric/util/config.h>
#include <faabric/util/environment.h>
#include <faabric/util/locks.h>
#include <faabric/util/timing.h>

#include <stdexcept>

#if (FAASM_SGX)
#include <sgx/SGXWAMRWasmModule.h>
#include <sgx/faasm_sgx_system.h>
#else
#include <storage/FileLoader.h>
#include <storage/FileSystem.h>
#endif

using namespace isolation;

namespace faaslet {

std::mutex flushMutex;

void preloadPythonRuntime()
{
    auto logger = faabric::util::getLogger();

    auto& conf = faabric::util::getSystemConfig();
    if (conf.pythonPreload != "on") {
        logger->info("Not preloading python runtime");
        return;
    }

    logger->info("Preparing python runtime");

    faabric::Message msg =
      faabric::util::messageFactory(PYTHON_USER, PYTHON_FUNC);
    msg.set_ispython(true);
    msg.set_pythonuser("python");
    msg.set_pythonfunction("noop");
    faabric::util::setMessageId(msg);

    faabric::scheduler::Scheduler& sch = faabric::scheduler::getScheduler();
    sch.callFunction(msg, true);
}

void Faaslet::flush()
{
    auto logger = faabric::util::getLogger();
    logger->debug("Faaslet {} flushing", id);

    // Note that all Faaslets on the given host will be flushing at the same
    // time, so we need to include some locking. They will also be killed
    // shortly after.
    // TODO avoid repeating global tidy-up that only needs to be done once
    faabric::util::UniqueLock lock(flushMutex);

    // Clear cached shared files
    storage::FileSystem::clearSharedFiles();

    // Clear cached wasm and object files
    storage::FileLoader& fileLoader = storage::getFileLoader();
    fileLoader.flushFunctionFiles();

    // Flush the module itself
    module->flush();
}

Faaslet::Faaslet(const faabric::Message& msg)
  : Executor(msg)
{
    faabric::util::SystemConfig& conf = faabric::util::getSystemConfig();

    // Instantiate the right wasm module for the chosen runtime
    if (conf.wasmVm == "wamr") {
#if (FAASM_SGX)
        // When SGX is enabled, we may still be running with vanilla WAMR
        if (msg.issgx()) {
            module = std::make_unique<wasm::SGXWAMRWasmModule>();
        } else {
            module = std::make_unique<wasm::WAMRWasmModule>();
        }
#else
        // Vanilla WAMR
        module = std::make_unique<wasm::WAMRWasmModule>();
#endif
    } else if (conf.wasmVm == "wavm") {
        module = std::make_unique<wasm::WAVMWasmModule>();
    } else {
        auto logger = faabric::util::getLogger();
        logger->error("Unrecognised wasm VM: {}", conf.wasmVm);
        throw std::runtime_error("Unrecognised wasm VM");
    }
}

int32_t Faaslet::executeTask(int threadPoolIdx,
                             int msgIdx,
                             std::shared_ptr<faabric::BatchExecuteRequest> req)
{
    // Lazily bind to function and isolate
    // Note that this has to be done within the executeTask function to be in
    // the same thread as the execution
    if (!module->isBound()) {
        // Add this thread to the cgroup
        CGroup cgroup(BASE_CGROUP_NAME);
        cgroup.addCurrentThread();
        std::string funcStr = faabric::util::funcToString(boundMessage, false);

        // Set up network namespace
        ns = claimNetworkNamespace();
        ns->addCurrentThread();

        module->bindToFunction(boundMessage);
    }

    int32_t returnValue = module->executeTask(threadPoolIdx, msgIdx, req);

    return returnValue;
}

void Faaslet::reset(const faabric::Message& msg)
{
    module->reset(msg);
}

void Faaslet::postFinish()
{
    ns->removeCurrentThread();
    returnNetworkNamespace(ns);
}

void Faaslet::restore(const faabric::Message& msg)
{
    auto logger = faabric::util::getLogger();

    auto& conf = faabric::util::getSystemConfig();
    const std::string snapshotKey = msg.snapshotkey();

    // Restore from snapshot if necessary
    if (conf.wasmVm == "wavm") {
        if (!snapshotKey.empty() && !msg.issgx()) {
            PROF_START(snapshotOverride)

            logger->debug("Restoring {} from snapshot {} before execution",
                          id,
                          snapshotKey);

            module->restore(snapshotKey);

            PROF_END(snapshotOverride)
        }
    }
}

FaasletFactory::~FaasletFactory() {}

std::shared_ptr<faabric::scheduler::Executor> FaasletFactory::createExecutor(
  const faabric::Message& msg)
{
    return std::make_shared<Faaslet>(msg);
}
}
