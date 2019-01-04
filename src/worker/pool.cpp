#include "worker.h"

#include <unistd.h>
#include <infra/infra.h>
#include <prof/prof.h>
#include <state/state.h>

#include <spdlog/spdlog.h>
#include <thread>


namespace worker {

    WorkerThreadPool::WorkerThreadPool() {
        // To run the pool we have two token pools, one for the total workers in the pool,
        // The other for prewarm workers. Workers first need to acquire a worker token,
        // then a prewarm token to say they should start
        util::SystemConfig &conf = util::getSystemConfig();
        prewarmTokenPool = new util::TokenPool(conf.prewarmTarget);
        workerTokenPool = new util::TokenPool(util::N_THREADS_PER_WORKER);

        // Add this worker to the list of workers accepting jobs
        infra::Redis *redis = infra::Redis::getThreadQueue();
        hostname = infra::Scheduler::getHostName();
        redis->sadd(infra::GLOBAL_WORKER_SET, hostname);
    }

    WorkerThreadPool::~WorkerThreadPool() {
        delete prewarmTokenPool;
        delete workerTokenPool;

        // Make sure this host is removed from the worker set
        infra::Redis *redis = infra::Redis::getThreadQueue();
        redis->srem(infra::GLOBAL_WORKER_SET, hostname);
    }

    void WorkerThreadPool::releasePrewarmToken(int prewarmToken) {
        prewarmTokenPool->releaseToken(prewarmToken);
    }

    void WorkerThreadPool::releaseWorkerToken(int workerIdx) {
        workerTokenPool->releaseToken(workerIdx);
    }

    int WorkerThreadPool::getWorkerToken() {
        return workerTokenPool->getToken();
    }

    int WorkerThreadPool::getPrewarmToken() {
        return prewarmTokenPool->getToken();
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
            int workerIdx = this->getWorkerToken();

            // See if we can prewarm
            int prewarmToken = this->getPrewarmToken();

            // Spawn thread to execute function
            std::thread funcThread([this, workerIdx, prewarmToken] {
                WorkerThread w(*this, workerIdx, prewarmToken);
                w.run();
            });

            funcThread.detach();
        }
    }

    StateThread::StateThread() = default;

    void StateThread::run() {
        state::State &s = state::getGlobalState();
        s.pushLoop();
    }

    WorkerThread::WorkerThread(WorkerThreadPool &threadPoolIn, int workerIdxIn,
                               int prewarmTokenIn) : threadPool(threadPoolIn),
                                                     workerIdx(workerIdxIn),
                                                     prewarmToken(prewarmTokenIn) {

        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        // Prepare host-specific variables
        id = infra::Scheduler::getHostName() + "_" + std::to_string(workerIdx);

        // Get Redis connection
        redis = infra::Redis::getThreadQueue();

        logger->debug("Starting worker {}", id);
        currentQueue = infra::Scheduler::getHostPrewarmQueue();
        this->initialise();
    }

    WorkerThread::~WorkerThread() {
        if (!_isInitialised) {
            return;
        }

        delete ns;
        delete module;
    }

    void WorkerThread::initialise() {
        // Set up network namespace
        isolationIdx = workerIdx + 1;
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

        // Notify scheduler
        infra::Scheduler::workerFinished(currentQueue);

        // Release token
        threadPool.releaseWorkerToken(workerIdx);
        threadPool.releasePrewarmToken(prewarmToken);
    }

    void WorkerThread::finishCall(message::Message &call, const std::string &errorMsg) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->info("Finished {}", infra::funcToString(call));

        bool isSuccess = errorMsg.empty();
        if (!isSuccess) {
            call.set_outputdata(errorMsg);
        }

        // Set result
        redis->setFunctionResult(call, isSuccess);

        // Restore the module memory after the execution
        module->restoreMemory();
    }

    void WorkerThread::bindToFunction(const message::Message &msg) {
        if (!_isInitialised) {
            throw std::runtime_error("Cannot bind without initialising");
        }

        if (_isBound) {
            throw std::runtime_error("Cannot bind worker thread again");
        }

        // Work out where to listen
        currentQueue = infra::Scheduler::getFunctionQueueName(msg);

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
                // Ignore and try again
            }
        }

        this->finish();
    }

    const std::string WorkerThread::processNextMessage() {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        // Work out which timeout
        int timeout = infra::Scheduler::getWorkerTimeout(currentQueue);

        // Wait for next message
        message::Message msg = redis->nextMessage(currentQueue, timeout);

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

        const util::TimePoint t = prof::startTimer();

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

        prof::logEndTimer("func-total", t);
        return empty;
    }
}
