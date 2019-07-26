#include "WorkerThread.h"

#include <system/CGroup.h>
#include <system/NetworkNamespace.h>

#include <scheduler/Scheduler.h>
#include <util/config.h>

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

        // Prewarm if necessary
        util::SystemConfig &conf = util::getSystemConfig();
        if (conf.prewarm == 0) {
            logger->debug("No prewarm for worker {}", id);
        } else {
            this->initialise();
        }
    }

    void WorkerThread::initialise() {
        if (_isInitialised) {
            throw std::runtime_error("Should not be initialising an already initialised thread");
        }

        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->debug("Initialising worker {}", id);

        // Set up network namespace
        isolationIdx = threadIdx + 1;
        std::string netnsName = BASE_NETNS_NAME + std::to_string(isolationIdx);
        ns = std::make_unique<NetworkNamespace>(netnsName);
        ns->addCurrentThread();

        // Add this thread to the cgroup
        CGroup cgroup(BASE_CGROUP_NAME);
        cgroup.addCurrentThread();

        // Initialise wasm module
        module = std::make_unique<wasm::WasmModule>();
        module->initialise();

        _isInitialised = true;
    }

    const bool WorkerThread::isInitialised() {
        return _isInitialised;
    }

    const bool WorkerThread::isBound() {
        return _isBound;
    }

    void WorkerThread::finish() {
        if (_isInitialised) {
            ns->removeCurrentThread();
        }

        if (_isBound) {
            // Notifiy scheduler if this thread was bound to a function
            scheduler.stopListeningToQueue(boundMessage);
        }
    }

    void WorkerThread::finishCall(message::Message &call, const std::string &errorMsg) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        const std::string funcStr = util::funcToString(call);
        logger->info("Finished {}", funcStr);

        // Decrement execution count
        scheduler.decrementExecutingCount();

        bool isSuccess = errorMsg.empty();
        if (!isSuccess) {
            call.set_outputdata(errorMsg);
        }

        // Set result
        logger->debug("Setting function result for {}", funcStr);
        globalBus.setFunctionResult(call, isSuccess);

        // Restore the module memory after the execution
        logger->debug("Restoring memory for {}", funcStr);
        const std::string snapshotKey = util::snapshotKeyForFunction(funcStr);
        module->restoreFullMemory(snapshotKey.c_str());
    }

    void WorkerThread::bindToFunction(const message::Message &msg) {
        if (!_isInitialised) {
            this->initialise();
        }

        if (_isBound) {
            throw std::runtime_error("Cannot bind worker thread more than once");
        }

        boundMessage = msg;

        // Set up with scheduler
        currentQueue = scheduler.listenToQueue(msg);

        // Perform the actual wasm initialisation
        module->bindToFunction(msg);

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

    const std::string WorkerThread::processNextMessage() {
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
            logger->info("Worker {} binding to {}", id, util::funcToString(msg));
            this->bindToFunction(msg);
        } else {
            errorMessage = this->executeCall(msg);
        }

        return errorMessage;
    }

    const std::string WorkerThread::executeCall(message::Message &call) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        const std::string funcStr = util::funcToString(call);
        logger->info("WorkerThread executing {}", funcStr);

        // Increment the execution count
        scheduler.incrementExecutingCount();

        // Snapshot the memory
        const std::string snapKey = util::snapshotKeyForFunction(funcStr);
        module->snapshotFullMemory(snapKey.c_str());

        // Create and execute the module
        try {
            module->execute(call);
        }
        catch (const std::exception &e) {
            std::string errorMessage = "Error: " + std::string(e.what());
            logger->error(errorMessage);

            this->finishCall(call, errorMessage);
            return errorMessage;
        }

        const std::string empty;
        this->finishCall(call, empty);

        return empty;
    }
}
