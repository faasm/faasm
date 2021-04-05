#include "Faaslet.h"

#include <stdexcept>
#include <system/CGroup.h>
#include <system/NetworkNamespace.h>

#include <faabric/scheduler/Scheduler.h>
#include <faabric/util/config.h>
#include <faabric/util/locks.h>
#include <faabric/util/timing.h>
#include <module_cache/WasmModuleCache.h>

#include <wamr/WAMRWasmModule.h>
#include <wavm/WAVMWasmModule.h>

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

    auto conf = faabric::util::getSystemConfig();
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

    // Note that all warm Faaslets on the given host will be flushing at the
    // same time, so we need to include some locking.
    // TODO avoid repeating global tidy-up that only needs to be done once
    faabric::util::UniqueLock lock(flushMutex);

    // Clear cached shared files
    storage::FileSystem::clearSharedFiles();

    // Clear cached wasm and object files
    storage::FileLoader& fileLoader = storage::getFileLoader();
    fileLoader.flushFunctionFiles();

    // Flush the module itself
    if (_isBound) {
        module->flush();
    }

    // Clear module cache on this host
    module_cache::getWasmModuleCache().clear();

    // Terminate this Faaslet
    throw faabric::util::ExecutorFinishedException("Faaslet flushed");
}

Faaslet::Faaslet(int threadIdxIn)
  : FaabricExecutor(threadIdxIn)
{
    // Set up network namespace
    isolationIdx = threadIdx + 1;
    std::string netnsName = BASE_NETNS_NAME + std::to_string(isolationIdx);
    ns = std::make_unique<NetworkNamespace>(netnsName);
    ns->addCurrentThread();

    // Add this thread to the cgroup
    CGroup cgroup(BASE_CGROUP_NAME);
    cgroup.addCurrentThread();
}

void Faaslet::postFinish()
{
    ns->removeCurrentThread();
}

void Faaslet::preFinishCall(faabric::Message& call,
                            bool success,
                            const std::string& errorMsg)
{
    const std::shared_ptr<spdlog::logger>& logger = faabric::util::getLogger();
    const std::string funcStr = faabric::util::funcToString(call, true);

    // Add captured stdout if necessary
    faabric::util::SystemConfig& conf = faabric::util::getSystemConfig();
    if (conf.captureStdout == "on") {
        std::string moduleStdout = module->getCapturedStdout();
        if (!moduleStdout.empty()) {
            std::string newOutput = moduleStdout + "\n" + call.outputdata();
            call.set_outputdata(newOutput);

            module->clearCapturedStdout();
        }
    }

    if (conf.wasmVm == "wavm") {
        logger->debug("Resetting module {} from zygote", funcStr);

        // Restore from zygote
        wasm::WAVMWasmModule& cachedModule =
          module_cache::getWasmModuleCache().getCachedModule(call);
        module = std::make_unique<wasm::WAVMWasmModule>(cachedModule);
    }
}

void Faaslet::postBind(const faabric::Message& msg, bool force)
{
    faabric::util::SystemConfig& conf = faabric::util::getSystemConfig();
    auto logger = faabric::util::getLogger();
    std::string funcStr = faabric::util::funcToString(msg, false);

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

        module->bindToFunction(msg);
    } else if (conf.wasmVm == "wavm") {
        // Get cached module
        wasm::WAVMWasmModule& cachedModule =
          module_cache::getWasmModuleCache().getCachedModule(msg);
        module = std::make_unique<wasm::WAVMWasmModule>(cachedModule);
    } else {
        logger->error("Unrecognised wasm VM: {}", conf.wasmVm);
        throw std::runtime_error("Unrecognised wasm VM");
    }
}

bool Faaslet::doExecute(faabric::Message& msg)
{
    auto logger = faabric::util::getLogger();

    auto conf = faabric::util::getSystemConfig();
    const std::string snapshotKey = msg.snapshotkey();

    // Restore from snapshot if necessary
    if (conf.wasmVm == "wavm") {
        if (!snapshotKey.empty() && !msg.issgx()) {
            PROF_START(snapshotOverride)

            logger->debug("Restoring {} from snapshot {} before execution",
                          id,
                          snapshotKey);
            module_cache::WasmModuleCache& registry =
              module_cache::getWasmModuleCache();

            wasm::WAVMWasmModule& snapshot = registry.getCachedModule(msg);
            module = std::make_unique<wasm::WAVMWasmModule>(snapshot);

            PROF_END(snapshotOverride)
        }
    }

    // Execute the function
    bool success = module->execute(msg);
    return success;
}
}
