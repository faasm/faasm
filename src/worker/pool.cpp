#include "worker.h"

#include <unistd.h>
#include <infra/infra.h>
#include <prof/prof.h>
#include <state/state.h>

#include <spdlog/spdlog.h>
#include <thread>


namespace worker {

    WorkerThreadPool::WorkerThreadPool(int nThreads, int nPrewarm) : threadTokenPool(nThreads),
                                                                     prewarmTokenPool(nPrewarm),
                                                                     redis(infra::Redis::getQueue()),
                                                                     scheduler(infra::getScheduler()) {
        // To run the pool we have two token pools, one for the total workers in the pool,
        // The other for prewarm workers. Workers first need to acquire a worker token,
        // then a prewarm token to say they should start

        // Ensure we can ping both redis instances
        infra::Redis::getQueue().ping();
        infra::Redis::getState().ping();

        // Notify scheduler
        scheduler.addCurrentHostToWorkerPool();
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

        // Add hostname to global workers
        scheduler.addCurrentHostToWorkerPool();
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

    std::string WorkerThreadPool::threadBound(const WorkerThread &thread) {
        // Notify the scheduler
        scheduler.workerBound(thread.boundMessage);

        // Release thread's prewarm token
        prewarmTokenPool.releaseToken(thread.prewarmToken);

        // Tell the thread where to listen
        std::string newQueue = scheduler.getFunctionQueueName(thread.boundMessage);
        return newQueue;
    }

    void WorkerThreadPool::threadFinished(WorkerThread &thread) {
        if (thread.isBound()) {
            // Notifiy scheduler if this thread was bound to a function
            scheduler.workerUnbound(thread.boundMessage);
        } else {
            // If thread was not bound, needs to release its prewarm token
            prewarmTokenPool.releaseToken(thread.prewarmToken);
        }

        // Release worker token
        threadTokenPool.releaseToken(thread.threadIdx);
    }

    std::string WorkerThreadPool::getPrewarmQueue() {
        return scheduler.getHostPrewarmQueue();
    }

    StateThread::StateThread() = default;

    void StateThread::run() {
        state::State &s = state::getGlobalState();
        s.pushLoop();
    }

    WorkerThread::WorkerThread(WorkerThreadPool &threadPoolIn, int threadIdxIn,
                               int prewarmTokenIn) : threadPool(threadPoolIn),
                                                     threadIdx(threadIdxIn),
                                                     prewarmToken(prewarmTokenIn),
                                                     redis(infra::Redis::getQueue()) {

        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        // Prepare host-specific variables
        id = util::getHostName() + "_" + std::to_string(threadIdx);

        logger->debug("Starting worker {}", id);
        currentQueue = threadPool.getPrewarmQueue();

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
        isolationIdx = threadIdx + 1;
        std::string netnsName = BASE_NETNS_NAME + std::to_string(isolationIdx);
        ns = new NetworkNamespace(netnsName);
        ns->addCurrentThread();

        // Add this thread to the cgroup
        CGroup cgroup(BASE_CGROUP_NAME);
        cgroup.addCurrentThread();

        // Initialise wasm module
        module = new wasm::WasmModule();
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

        // Notify the pool
        threadPool.threadFinished(*this);
    }

    void WorkerThread::finishCall(message::Message &call, const std::string &errorMsg) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->info("Finished {}", infra::funcToString(call));

        bool isSuccess = errorMsg.empty();
        if (!isSuccess) {
            call.set_outputdata(errorMsg);
        }

        // Set result
        redis.setFunctionResult(call, isSuccess);

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

        // Notify the pool
        boundMessage = msg;
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
            catch (infra::RedisNoResponseException &e) {
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
        message::Message msg = redis.nextMessage(currentQueue, timeout);

        // Handle the message
        std::string errorMessage;
        if (msg.type() == message::Message_MessageType_BIND) {
            logger->info("Worker {} binding to {}", id, infra::funcToString(msg));
            this->bindToFunction(msg);
        } else {
            errorMessage = this->executeCall(msg);
        }

        return errorMessage;
    }

    const std::string WorkerThread::executeCall(message::Message &call) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        logger->info("WorkerThread executing {}", infra::funcToString(call));

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
