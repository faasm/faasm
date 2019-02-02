#include "WorkerThreadPool.h"
#include "StateThread.h"
#include "CGroup.h"

#include <unistd.h>
#include <scheduler/Scheduler.h>
#include <prof/prof.h>

#include <spdlog/spdlog.h>
#include <thread>


namespace worker {

    WorkerThreadPool::WorkerThreadPool(int nThreads, int nPrewarm) : threadTokenPool(nThreads),
                                                                     prewarmTokenPool(nPrewarm),
                                                                     scheduler(scheduler::getScheduler()) {
        // To run the pool we have two token pools, one for the total workers in the pool,
        // The other for prewarm workers. Workers first need to acquire a worker token,
        // then a prewarm token to say they should start

        // Ensure we can ping both redis instances
        redis::Redis::getQueue().ping();
        redis::Redis::getState().ping();
    }

    void WorkerThreadPool::start() {
        // Spawn the state thread first if not running in full async
        util::SystemConfig &conf = util::getSystemConfig();
        if (!conf.fullAsync) {
            std::thread stateThread([] {
                StateThread s;
                s.run();
            });
            stateThread.detach();
        }

        // Spawn worker threads until we've hit the worker limit, thus creating a pool
        // that will replenish when one releases its token
        while (true) {
            // Try to get an available slot (blocks if none available)
            int threadIdx = this->getThreadToken();

            // See if we can prewarm
            int prewarmToken = this->getPrewarmToken();

            // Spawn thread to execute function
            std::thread funcThread([this, threadIdx, prewarmToken] {
                WorkerThread w(*this, threadIdx, prewarmToken);
                w.run();
            });

            funcThread.detach();
        }
    }

    void WorkerThreadPool::reset() {
        threadTokenPool.reset();
        prewarmTokenPool.reset();
    }

    int WorkerThreadPool::getPrewarmToken() {
        return prewarmTokenPool.getToken();
    }

    int WorkerThreadPool::getThreadToken() {
        return threadTokenPool.getToken();
    }

    int WorkerThreadPool::getPrewarmCount() {
        return prewarmTokenPool.taken();
    }

    int WorkerThreadPool::getThreadCount() {
        return threadTokenPool.taken();
    }

    scheduler::InMemoryMessageQueue *WorkerThreadPool::getBindQueue() {
        return scheduler.getBindQueue();
    }

    scheduler::InMemoryMessageQueue *WorkerThreadPool::threadBound(const WorkerThread &thread) {
        // Release thread's prewarm token
        prewarmTokenPool.releaseToken(thread.prewarmToken);

        // Notify scheduler
        return scheduler.listenToQueue(thread.boundMessage);
    }

    void WorkerThreadPool::callFinished(message::Message &msg, bool isSuccess) {
        scheduler::MessageQueue::getGlobalQueue().setFunctionResult(msg, isSuccess);
    }

    void WorkerThreadPool::threadFinished(WorkerThread &thread) {
        if (thread.isBound()) {
            // Notifiy scheduler if this thread was bound to a function
            scheduler.stopListeningToQueue(thread.boundMessage);
        } else {
            // If thread was not bound, needs to release its prewarm token
            prewarmTokenPool.releaseToken(thread.prewarmToken);
        }

        // Release worker token
        threadTokenPool.releaseToken(thread.threadIdx);
    }

    WorkerThread::WorkerThread(WorkerThreadPool &threadPoolIn, int threadIdxIn,
                               int prewarmTokenIn) : threadPool(threadPoolIn),
                                                     threadIdx(threadIdxIn),
                                                     prewarmToken(prewarmTokenIn) {

        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        // Prepare host-specific variables
        id = util::getHostName() + "_" + std::to_string(threadIdx);

        logger->debug("Starting worker {}", id);

        // Listen to bind queue by default
        currentQueue = threadPool.getBindQueue();

        // If we've got a prewarm token less than zero we don't need to prewarm
        if (prewarmTokenIn < 0) {
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

        // Notify the pool
        threadPool.threadFinished(*this);
    }

    void WorkerThread::finishCall(message::Message &call, const std::string &errorMsg) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->info("Finished {}", util::funcToString(call));

        bool isSuccess = errorMsg.empty();
        if (!isSuccess) {
            call.set_outputdata(errorMsg);
        }

        // Set result
        threadPool.callFinished(call, isSuccess);

        // Restore the module memory after the execution
        module->restoreMemory();
    }

    void WorkerThread::bindToFunction(const message::Message &msg) {
        if (!_isInitialised) {
            // If we're in "no prewarm" mode, then we need to initialise on demand
            if (prewarmToken < 0) {
                this->initialise();
            } else {
                // If we get here, something has gone wrong
                throw std::runtime_error("Cannot bind without initialising");
            }
        }

        if (_isBound) {
            throw std::runtime_error("Cannot bind worker thread more than once");
        }

        boundMessage = msg;

        // Set up with scheduler
        currentQueue = threadPool.threadBound(*this);

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
