#include "Faaslet.h"

#include <system/CGroup.h>
#include <system/NetworkNamespace.h>

#include <faabric/scheduler/Scheduler.h>
#include <faabric/util/config.h>
#include <faabric/util/timing.h>
#include <module_cache/WasmModuleCache.h>

#include <wavm/WAVMWasmModule.h>

#if(FAASM_SGX == 1)
#include <sgx/SGXWAMRWasmModule.h>
#include <sgx/sgx_system.h>
#else

#include <wamr/WAMRWasmModule.h>

#endif

using namespace isolation;

namespace faaslet {
    void flushFaasletHost() {
        const std::shared_ptr<spdlog::logger> &logger = faabric::util::getLogger();
        logger->warn("Flushing host {}", faabric::util::getSystemConfig().endpointHost);

        // Clear out any cached state
        faabric::state::getGlobalState().forceClearAll(false);

        // Clear shared files
        storage::FileSystem::clearSharedFiles();

        // Reset scheduler
        faabric::scheduler::Scheduler &sch = faabric::scheduler::getScheduler();
        sch.clear();
        sch.addHostToGlobalSet();

        // Clear zygotes
        module_cache::getWasmModuleCache().clear();
    }

    void Faaslet::flush() {
        flushFaasletHost();
    }

    Faaslet::Faaslet(int threadIdxIn) : FaabricExecutor(threadIdxIn) {
        const std::shared_ptr<spdlog::logger> &logger = faabric::util::getLogger();

        // Set up network namespace
        isolationIdx = threadIdx + 1;
        std::string netnsName = BASE_NETNS_NAME + std::to_string(isolationIdx);
        ns = std::make_unique<NetworkNamespace>(netnsName);
        ns->addCurrentThread();

        // Add this thread to the cgroup
        CGroup cgroup(BASE_CGROUP_NAME);
        cgroup.addCurrentThread();
    }

    void Faaslet::postFinish() {
        ns->removeCurrentThread();
    }

    void Faaslet::postFinishCall(faabric::Message &call, bool success, const std::string &errorMsg) {
        const std::shared_ptr<spdlog::logger> &logger = faabric::util::getLogger();
        const std::string funcStr = faabric::util::funcToString(msg, true);

        // Add captured stdout if necessary
        faabric::util::SystemConfig &conf = faabric::util::getSystemConfig();
        if (conf.captureStdout == "on") {
            std::string moduleStdout = module->getCapturedStdout();
            if (!moduleStdout.empty()) {
                std::string newOutput = moduleStdout + "\n" + call.outputdata();
                call.set_outputdata(newOutput);

                module->clearCapturedStdout();
            }
        }

        if (conf.wasmVm == "wavm") {
            // Restore from zygote
            logger->debug("Resetting module {} from zygote", funcStr);
            module_cache::WasmModuleCache &registry = module_cache::getWasmModuleCache();
            wasm::WAVMWasmModule &cachedModule = registry.getCachedModule(call);

            auto *wavmModulePtr = dynamic_cast<wasm::WAVMWasmModule *>(module.get());
            *wavmModulePtr = cachedModule;
        }

    }

    void Faaslet::postBind(const faabric::Message &msg, bool force) {
        faabric::util::SystemConfig &conf = faabric::util::getSystemConfig();

        // Instantiate the right wasm module for our chosen runtime
        if (conf.wasmVm == "wamr") {
#if(FAASM_SGX == 1)
            sgx_enclave_id_t enclaveId = sgx::getGlobalEnclaveId();
            module = std::make_unique<wasm::SGXWAMRWasmModule>(enclaveId);
#else
            module = std::make_unique<wasm::WAMRWasmModule>();
#endif
            module->bindToFunction(msg);
        } else {
            // Instantiate a WAVM module from its snapshot
            PROF_START(snapshotRestore)

            module_cache::WasmModuleCache &registry = module_cache::getWasmModuleCache();
            wasm::WAVMWasmModule &snapshot = registry.getCachedModule(msg);
            module = std::make_unique<wasm::WAVMWasmModule>(snapshot);

            PROF_END(snapshotRestore)
        }
    }

    /**
     * NOTE - this is a complete override of the Faabric version
     */
    std::string Faaslet::processNextMessage() {
        const std::shared_ptr<spdlog::logger> &logger = faabric::util::getLogger();

        // Work out which timeout
        int timeoutMs;
        faabric::util::SystemConfig conf = faabric::util::getSystemConfig();
        if (_isBound) {
            timeoutMs = conf.boundTimeout;
        } else {
            timeoutMs = conf.unboundTimeout;
        }

        // Wait for next message (note, timeout in ms)
        faabric::Message msg = currentQueue->dequeue(timeoutMs);

        // Handle the message
        std::string errorMessage;
        if (msg.isflushrequest()) {
            // Clear out this worker host if we've received a flush message
            flushFaasletHost();

            scheduler.preflightPythonCall();
        } else if (msg.type() == faabric::Message_MessageType_BIND) {
            const std::string funcStr = faabric::util::funcToString(msg, false);
            logger->info("Faaslet {} binding to {}", id, funcStr);

            try {
                this->bindToFunction(msg);
            } catch (faabric::util::InvalidFunctionException &e) {
                errorMessage = "Invalid function: " + funcStr;
            }
        } else {
            int coldStartInterval = msg.coldstartinterval();
            bool isColdStart = coldStartInterval > 0 &&
                               executionCount > 0 &&
                               executionCount % coldStartInterval == 0;

            if (isColdStart) {
                const std::string funcStr = faabric::util::funcToString(msg, true);
                logger->debug("Forcing cold start of {} ({}/{})", funcStr, executionCount, coldStartInterval);

                // Bind to function again
                this->bindToFunction(msg, true);
            }

            // Check if we need to restore from a different snapshot
            if (conf.wasmVm == "wavm") {
                const std::string snapshotKey = msg.snapshotkey();
                if (!snapshotKey.empty() && !msg.issgx()) {
                    PROF_START(snapshotOverride)

                    module_cache::WasmModuleCache &registry = module_cache::getWasmModuleCache();
                    wasm::WAVMWasmModule &snapshot = registry.getCachedModule(msg);
                    module = std::make_unique<wasm::WAVMWasmModule>(snapshot);

                    PROF_END(snapshotOverride)
                }
            }

            // Do the actual execution
            errorMessage = this->executeCall(msg);
        }

        return errorMessage;
    }

    std::string Faaslet::doExecute(const faabric::Message &msg) {
        bool success = module->execute(call);
        return success;
    }
}