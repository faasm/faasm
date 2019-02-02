#include "WorkerThread.h"

#include "CGroup.h"

#include <scheduler/Scheduler.h>
#include <util/config.h>
#include <prof/prof.h>


namespace worker {
    WorkerThread::WorkerThread(int threadIdxIn) : threadIdx(threadIdxIn),
                                                  scheduler(scheduler::getScheduler()) {

        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        // Prepare host-specific variables
        id = util::getHostName() + "_" + std::to_string(threadIdx);

        logger->debug("Starting worker {}", id);

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

    WorkerThread::~WorkerThread() {
        if (!_isInitialised) {
            return;
        }

        delete ns;
        delete module;
    }

    void WorkerThread::initialise() {
        if (_isInitialised) {
            throw std::runtime_error("Should not be initialising an already initialised thread");
        }

        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->debug("Initialising worker {}", id);

        // Set up network namespace
        const util::TimePoint &isolationTs = prof::startTimer();
        isolationIdx = threadIdx + 1;
        std::string netnsName = BASE_NETNS_NAME + std::to_string(isolationIdx);
        ns = new NetworkNamespace(netnsName);
        ns->addCurrentThread();

        // Add this thread to the cgroup
        CGroup cgroup(BASE_CGROUP_NAME);
        cgroup.addCurrentThread();
        prof::logEndTimer("isolation", isolationTs);

        // Initialise wasm module
        const util::TimePoint &wavmTs = prof::startTimer();
        module = new wasm::WasmModule();
        module->initialise();
        prof::logEndTimer("wavm-init", wavmTs);

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
        logger->info("Finished {}", util::funcToString(call));

        bool isSuccess = errorMsg.empty();
        if (!isSuccess) {
            call.set_outputdata(errorMsg);
        }

        // Set result
        scheduler.getGlobalQueue().setFunctionResult(call, isSuccess);

        // Restore the module memory after the execution
        module->restoreMemory();
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
        // Wait for next message
        while (true) {
            try {
                std::string errorMessage = this->processNextMessage();

                // Drop out if there's some issue
                if (!errorMessage.empty()) {
                    break;
                }
            }
            catch (redis::RedisNoResponseException &e) {
                // At this point we've received no message, so die off
                break;
            }
        }

        this->finish();
    }

    const std::string WorkerThread::processNextMessage() {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        // Work out which timeout
        int timeout;
        util::SystemConfig conf = util::getSystemConfig();
        if (_isBound) {
            timeout = conf.boundTimeout;
        } else {
            timeout = conf.unboundTimeout;
        }

        // Wait for next message
        message::Message msg = currentQueue->dequeue(timeout);

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

        logger->info("WorkerThread executing {}", util::funcToString(call));

        // Create and execute the module
        wasm::CallChain callChain(call);
        try {
            module->execute(call, callChain);
        }
        catch (const std::exception &e) {
            std::string errorMessage = "Error: " + std::string(e.what());
            logger->error(errorMessage);

            this->finishCall(call, errorMessage);
            return errorMessage;
        }

        // Process any chained calls
        std::string chainErrorMessage = callChain.execute();
        if (!chainErrorMessage.empty()) {
            this->finishCall(call, chainErrorMessage);
            return chainErrorMessage;
        }

        const std::string empty;
        this->finishCall(call, empty);

        return empty;
    }
}
