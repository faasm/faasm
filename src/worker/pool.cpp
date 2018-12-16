#include "worker.h"

#include <unistd.h>
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

        // TODO do this without delaying start
        // Introduce a small delay to avoid race condition
        uint microseconds = (uint) workerIdx * 50 * 1000;
        usleep(microseconds);

        const std::string hostname = util::getEnvVar("HOSTNAME", "");
        id = hostname + "_" + std::to_string(workerIdx);

        // Get Redis connection
        queue = infra::Redis::getThreadQueue();

        // If we need more prewarm containers, set this worker to be prewarm. If not, sit in cold queue
        bool shouldPrewarm = infra::Scheduler::prewarmWorker();
        if (shouldPrewarm) {
            logger->debug("Prewarming worker {}", id);

            currentQueue = infra::PREWARM_QUEUE;
            this->initialise();
        } else {
            logger->debug("Cold starting worker {}", id);

            currentQueue = infra::COLD_QUEUE;
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
        queue->setFunctionResult(call, isSuccess);

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

        // Inform scheduler
        currentQueue = infra::Scheduler::workerPrewarmToBound(msg);

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
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        // Work out which timeout
        int timeout = infra::Scheduler::getWorkerTimeout(currentQueue);

        // Wait for next message
        message::Message msg = queue->nextMessage(currentQueue, timeout);

        // Handle the message
        std::string errorMessage;
        if (msg.type() == message::Message_MessageType_BIND) {
            logger->info("Worker {} binding to {}", id, infra::funcToString(msg));
            this->bindToFunction(msg);
        } else if (msg.type() == message::Message_MessageType_PREWARM) {
            logger->info("Worker {} prewarming", id);
            currentQueue = infra::Scheduler::workerColdToPrewarm();
            this->initialise();
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
