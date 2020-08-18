#include "Faaslet.h"

#include <system/CGroup.h>
#include <system/NetworkNamespace.h>

#include <scheduler/Scheduler.h>
#include <util/config.h>
#include <util/timing.h>
#include <module_cache/WasmModuleCache.h>

#include <wavm/WAVMWasmModule.h>

#if(FAASM_SGX == 1)
#include <sgx/SGXWAMRWasmModule.h>

extern "C"{
extern sgx_enclave_id_t enclave_id;
};
#else
#include <wamr/WAMRWasmModule.h>
#endif

using namespace isolation;

namespace faaslet {
    void flushFaasletHost() {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->warn("Flushing host {}", util::getSystemConfig().endpointHost);

        // Clear out any cached state
        state::getGlobalState().forceClearAll(false);

        // Clear shared files
        storage::FileSystem::clearSharedFiles();

        // Reset scheduler
        scheduler::Scheduler &sch = scheduler::getScheduler();
        sch.clear();
        sch.addHostToGlobalSet();

        // Clear out global message bus
        scheduler::getGlobalMessageBus().clear();

        // Clear zygotes
        module_cache::getWasmModuleCache().clear();
    }

    Faaslet::Faaslet(int threadIdxIn) : threadIdx(threadIdxIn),
                                        scheduler(scheduler::getScheduler()),
                                        globalBus(scheduler::getGlobalMessageBus()) {

        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        // Set an ID for this Faaslet
        id = util::getSystemConfig().endpointHost + "_" + std::to_string(threadIdx);

        logger->debug("Starting worker thread {}", id);

        // Listen to bind queue by default
        currentQueue = scheduler.getBindQueue();

        // Set up network namespace
        isolationIdx = threadIdx + 1;
        std::string netnsName = BASE_NETNS_NAME + std::to_string(isolationIdx);
        ns = std::make_unique<NetworkNamespace>(netnsName);
        ns->addCurrentThread();

        // Add this thread to the cgroup
        CGroup cgroup(BASE_CGROUP_NAME);
        cgroup.addCurrentThread();
    }

    const bool Faaslet::isBound() {
        return _isBound;
    }

    void Faaslet::finish() {
        ns->removeCurrentThread();

        if (_isBound) {
            // Notify scheduler if this thread was bound to a function
            scheduler.notifyFaasletFinished(boundMessage);
        }
    }

    void Faaslet::finishCall(message::Message &call, bool success, const std::string &errorMsg) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        const std::string funcStr = util::funcToString(call, true);
        logger->info("Finished {}", funcStr);
        if (!success) {
            call.set_outputdata(errorMsg);
        }

        // Add captured stdout if necessary
        util::SystemConfig &conf = util::getSystemConfig();

        if (conf.captureStdout == "on") {
            std::string moduleStdout = module->getCapturedStdout();
            if (!moduleStdout.empty()) {
                std::string newOutput = moduleStdout + "\n" + call.outputdata();
                call.set_outputdata(newOutput);

                module->clearCapturedStdout();
            }
        }

        fflush(stdout);

        // Notify the scheduler *before* setting the result. Calls awaiting
        // the result will carry on blocking
        scheduler.notifyCallFinished(call);

        // Set result
        logger->debug("Setting function result for {}", funcStr);
        globalBus.setFunctionResult(call);

        if (conf.wasmVm == "wavm") {
            // Restore from zygote
            logger->debug("Resetting module {} from zygote", funcStr);
            module_cache::WasmModuleCache &registry = module_cache::getWasmModuleCache();
            wasm::WAVMWasmModule &cachedModule = registry.getCachedModule(call);

            auto *wavmModulePtr = dynamic_cast<wasm::WAVMWasmModule *>(module.get());
            *wavmModulePtr = cachedModule;
        }

        // Increment the execution counter
        executionCount++;
    }

    void Faaslet::bindToFunction(const message::Message &msg, bool force) {
        // If already bound, will be an error, unless forced to rebind to the same message
        if (_isBound) {
            if (force) {
                if (msg.user() != boundMessage.user() || msg.function() != boundMessage.function()) {
                    throw std::runtime_error("Cannot force bind to a different function");
                }
            } else {
                throw std::runtime_error("Cannot bind worker thread more than once");
            }
        }

        boundMessage = msg;

        // Get queue from the scheduler
        currentQueue = scheduler.getFunctionQueue(msg);

        util::SystemConfig &conf = util::getSystemConfig();

        // Instantiate the right wasm module for our chosen runtime
        if(conf.wasmVm == "wamr") {
#if(FAASM_SGX == 1)
            module = std::make_unique<wasm::SGXWAMRWasmModule>(&enclave_id);
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

        _isBound = true;
    }

    void Faaslet::run() {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        // Wait for next message
        while (true) {
            try {
                logger->debug("Faaslet {} waiting for next message", this->id);
                std::string errorMessage = this->processNextMessage();

                // Drop out if there's some issue
                if (!errorMessage.empty()) {
                    break;
                }
            }
            catch (util::QueueTimeoutException &e) {
                // At this point we've received no message, so die off
                logger->debug("Faaslet {} got no messages. Finishing", this->id);
                break;
            }
        }

        this->finish();
    }

    std::string Faaslet::processNextMessage() {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        // Work out which timeout
        int timeoutMs;
        util::SystemConfig conf = util::getSystemConfig();
        if (_isBound) {
            timeoutMs = conf.boundTimeout;
        } else {
            timeoutMs = conf.unboundTimeout;
        }

        // Wait for next message (note, timeout in ms)
        message::Message msg = currentQueue->dequeue(timeoutMs);

        // Handle the message
        std::string errorMessage;
        if (msg.isflushrequest()) {
            // Clear out this worker host if we've received a flush message
            flushFaasletHost();

            scheduler.preflightPythonCall();
        } else if (msg.type() == message::Message_MessageType_BIND) {
            const std::string funcStr = util::funcToString(msg, false);
            logger->info("Faaslet {} binding to {}", id, funcStr);

            try {
                this->bindToFunction(msg);
            } catch (util::InvalidFunctionException &e) {
                errorMessage = "Invalid function: " + funcStr;
            }
        } else {
            int coldStartInterval = msg.coldstartinterval();
            bool isColdStart = coldStartInterval > 0 &&
                               executionCount > 0 &&
                               executionCount % coldStartInterval == 0;

            if (isColdStart) {
                const std::string funcStr = util::funcToString(msg, true);
                logger->debug("Forcing cold start of {} ({}/{})", funcStr, executionCount, coldStartInterval);

                // Bind to function again
                this->bindToFunction(msg, true);
            }

            // Check if we need to restore from a different snapshot
            if(conf.wasmVm == "wavm") {
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

    std::string Faaslet::executeCall(message::Message &call) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        const std::string funcStr = util::funcToString(call, true);
        logger->info("Faaslet executing {}", funcStr);

        // Create and execute the module
        bool success;
        std::string errorMessage;
        try {
            success = module->execute(call);
        }
        catch (const std::exception &e) {
            errorMessage = "Error: " + std::string(e.what());
            logger->error(errorMessage);
            success = false;
            call.set_returnvalue(1);
        }

        if (!success && errorMessage.empty()) {
            errorMessage = "Call failed (return value=" + std::to_string(call.returnvalue()) + ")";
        }

        this->finishCall(call, success, errorMessage);
        return errorMessage;
    }
}