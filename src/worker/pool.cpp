#include "worker.h"

#include <infra/infra.h>
#include <prof/prof.h>

#include <spdlog/spdlog.h>
#include <thread>


namespace worker {
    static util::TokenPool tokenPool(util::N_THREADS_PER_WORKER);

    void startWorkerThreadPool() {
        // Spawn worker threads until we've hit the limit (thus creating a pool that will replenish
        // when one releases its token)
        while (true) {
            // Try to get an available slot (blocks if none available)
            int workerIdx = tokenPool.getToken();

            // Spawn thread to execute function
            std::thread funcThread([workerIdx] {
                WorkerThread w(workerIdx);
                w.run();
            });

            funcThread.detach();
        }
    }

    WorkerThread::WorkerThread(int workerIdx) : workerIdx(workerIdx) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        const std::string hostname = util::getEnvVar("HOSTNAME", "");
        id = hostname + "_" + std::to_string(workerIdx);

        logger->debug("Cold starting worker {}", id);

        // Get Redis connection
        redis = infra::Redis::getThreadConnection();

        // If we need more prewarm containers, set this worker to be prewarm.
        // If not, sit in cold queue
        util::SystemConfig conf = util::getSystemConfig();
        long prewarmCount = redis->scard(infra::PREWARM_SET);
        if(prewarmCount < conf.prewarm_target) {
            this->initialise();
        }
        else {
            // Add to cold
            this->updateQueue(infra::COLD_QUEUE, infra::COLD_SET);
        }
    }

    WorkerThread::~WorkerThread() {
        // Nothing to do if not initialised
        if(!_isInitialised) {
            return;
        }

        delete ns;
        delete module;
    }

    void WorkerThread::updateQueue(const std::string &queueName, const std::string &setName) {
        // Remove from current queue if necessary
        if(!currentQueue.empty()) {
            redis->srem(currentSet, id);
        }

        currentQueue = queueName;
        currentSet = setName;

        // Add to new queue
        redis->sadd(currentSet, id);
    }

    void WorkerThread::initialise() {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        // Check there is still space in the pre-warm set
        long prewarmCount = redis->scard(infra::PREWARM_SET);
        util::SystemConfig conf = util::getSystemConfig();
        if(prewarmCount >= conf.prewarm_target) {
            logger->debug("Ignoring pre-warm for worker {}, already {}", id, prewarmCount);
        }

        // Add to prewarm
        logger->debug("Prewarming worker {}", id);
        this->updateQueue(infra::PREWARM_QUEUE, infra::PREWARM_SET);

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
        if(_isInitialised) {
            ns->removeCurrentThread();
        }

        // Remove from set
        redis->srem(currentSet, id);

        tokenPool.releaseToken(workerIdx);
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
        // module->restoreCleanMemory();
    }

    void WorkerThread::bindToFunction(const message::Message &msg) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        if(!_isInitialised) {
            throw std::runtime_error("Cannot bind without initialising");
        }

        if(_isBound) {
            throw std::runtime_error("Cannot bind worker thread again");
        }

        // Check if the target has already been reached, in which case ignore
        const std::string newSet = infra::getFunctionSetName(msg);
        long currentCount = redis->scard(newSet);
        if (currentCount >= msg.target()) {
            logger->info("Worker {} ignoring bind to {}, already {}", id, infra::funcToString(msg), currentCount);
            return;
        }

        // Bind to new function
        logger->info("Worker {} binding to {}", id, infra::funcToString(msg));
        this->updateQueue(infra::getFunctionQueueName(msg), newSet);

        // Ask a cold worker to take place in the pre-warm pool
        message::Message prewarmMsg = infra::buildPrewarmMessage(msg);
        redis->enqueueMessage(infra::COLD_QUEUE, prewarmMsg);

        // Perform the actual wasm initialisation
        module->bindToFunction(msg);

        _isBound = true;
    }

    void WorkerThread::run() {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

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
                logger->debug("No messages received in timeout");

                this->finish();
                return;
            }
        }

        this->finish();
    }

    const std::string WorkerThread::processNextMessage() {
        // Work out which timeout
        int timeout;
        util::SystemConfig conf = util::getSystemConfig();
        if (currentQueue == infra::COLD_QUEUE || currentQueue == infra::PREWARM_QUEUE) {
            timeout = conf.unbound_timeout;
        } else {
            timeout = conf.bound_timeout;
        }

        // Wait for next message
        message::Message msg = redis->nextMessage(currentQueue, timeout);

        // Handle the message
        std::string errorMessage;
        if (msg.type() == message::Message_MessageType_BIND) {
            // Binding
            this->bindToFunction(msg);
        }
        else if (msg.type() == message::Message_MessageType_PREWARM) {
            // Pre-warm
            this->initialise();
        } else {
            errorMessage = this->executeCall(msg);
        }

        return errorMessage;
    }

    void WorkerThread::runSingle() {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        std::string errorMessage = this->processNextMessage();

        if (!errorMessage.empty()) {
            logger->error("WorkerThread failed with error: {}", errorMessage);
        }

        this->finish();
    }

    const std::string WorkerThread::executeCall(message::Message &call) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        const std::chrono::steady_clock::time_point &t = prof::startTimer();

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
