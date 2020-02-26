#include "WorkerThread.h"

#include <system/CGroup.h>
#include <system/NetworkNamespace.h>

#include <scheduler/Scheduler.h>
#include <util/config.h>
#include <util/timing.h>
#include <zygote/ZygoteRegistry.h>

using namespace isolation;

namespace worker {
    WorkerThread::WorkerThread(int threadIdxIn) : threadIdx(threadIdxIn),
                                                  scheduler(scheduler::getScheduler()),
                                                  globalBus(scheduler::getGlobalMessageBus()) {

        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        // Prepare node-specific variables
        id = util::getNodeId() + "_" + std::to_string(threadIdx);

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

        // Zero the execution counter
        executionCount = 0;
    }

    const bool WorkerThread::isBound() {
        return _isBound;
    }

    void WorkerThread::finish() {
        ns->removeCurrentThread();

        if (_isBound) {
            // Notify scheduler if this thread was bound to a function
            scheduler.notifyThreadFinished(boundMessage);
        }
    }

    void WorkerThread::finishCall(message::Message &call, const std::string &errorMsg) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        const std::string funcStr = util::funcToString(call, true);
        logger->info("Finished {}", funcStr);

        bool isSuccess = errorMsg.empty();
        if (!isSuccess) {
            call.set_outputdata(errorMsg);
        }

        // Add captured stdout if necessary
        util::SystemConfig &conf = util::getSystemConfig();
        if (conf.captureStdout == "on") {
            std::string moduleStdout = module->getCapturedStdout();
            if(!moduleStdout.empty()) {
                std::string newOutput = moduleStdout + "\n" + call.outputdata();
                call.set_outputdata(newOutput);

                module->clearCapturedStdout();
            }
        }

        // Notify the scheduler *before* setting the result
        scheduler.notifyCallFinished(call);

        // Set result
        logger->debug("Setting function result for {}", funcStr);
        globalBus.setFunctionResult(call, isSuccess);

        // Restore from zygote
        logger->debug("Resetting module {} from zygote", funcStr);
        zygote::ZygoteRegistry &registry = zygote::getZygoteRegistry();
        wasm::WasmModule &zygote = registry.getZygote(call);
        *module = zygote;

        // Increment the execution counter
        executionCount++;
    }

    void WorkerThread::bindToFunction(const message::Message &msg, bool force) {
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

        // Instantiate the module from its zygote
        PROF_START(zygoteCreate)

        zygote::ZygoteRegistry &registry = zygote::getZygoteRegistry();
        wasm::WasmModule &zygote = registry.getZygote(msg);
        module = std::make_unique<wasm::WasmModule>(zygote);

        PROF_END(zygoteCreate)

        _isBound = true;
    }

    void WorkerThread::run() {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        // Wait for next message
        while (true) {
            try {
                logger->debug("Worker {} waiting for next message", this->id);
                std::string errorMessage = this->processNextMessage();

                // Drop out if there's some issue
                if (!errorMessage.empty()) {
                    break;
                }
            }
            catch (util::QueueTimeoutException &e) {
                // At this point we've received no message, so die off
                logger->debug("Worker {} got no messages. Finishing", this->id);
                break;
            }
        }

        this->finish();
    }

    std::string WorkerThread::processNextMessage() {
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
        if (msg.type() == message::Message_MessageType_BIND) {
            const std::string funcStr = util::funcToString(msg, false);
            logger->info("Worker {} binding to {}", id, funcStr);

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

            errorMessage = this->executeCall(msg);
        }

        return errorMessage;
    }

    const std::string WorkerThread::executeCall(message::Message &call) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        const std::string funcStr = util::funcToString(call, true);
        logger->info("WorkerThread executing {}", funcStr);

        // Create and execute the module
        int exitCode = 0;
        try {
            exitCode = module->execute(call);
        }
        catch (const std::exception &e) {
            std::string errorMessage = "Error: " + std::string(e.what());
            logger->error(errorMessage);

            this->finishCall(call, errorMessage);
            return errorMessage;
        }

        std::string errorMessage;

        if (exitCode != 0) {
            errorMessage = "Non-zero exit code: " + std::to_string(exitCode);
        }

        this->finishCall(call, errorMessage);
        return errorMessage;
    }
}
