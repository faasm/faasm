#include "worker.h"

#include <infra/infra.h>
#include <prof/prof.h>

#include <spdlog/spdlog.h>
#include <thread>


namespace worker {
    // TODO - must match the underlying number of available namespaces. Good to decouple?
    static int N_THREADS = 40;

    static int UNBOUND_TIMEOUT = 120;
    static int BOUND_TIMEOUT = 30;

    static util::TokenPool tokenPool(N_THREADS);

    void startWorkerPool() {
        // Spawn threads until we've hit the limit (effectively creating a thread pool)
        while (true) {
            // Try to get an available slot (blocks if none available)
            int workerIdx = tokenPool.getToken();

            // Spawn thread to execute function
            std::thread funcThread([workerIdx] {
                Worker w(workerIdx);
                w.run();
            });

            funcThread.detach();
        }
    }

    Worker::Worker(int workerIdx) : workerIdx(workerIdx) {
        const std::string hostname = util::getEnvVar("HOSTNAME", "");
        queueName = hostname + "_" + std::to_string(workerIdx);

        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->debug("Starting worker {} on queue {}", workerIdx, queueName);

        // Get Redis connection before we isolate ourselves
        redis = infra::Redis::getThreadConnection();

        // Set up network namespace
        isolationIdx = workerIdx + 1;
        std::string netnsName = BASE_NETNS_NAME + std::to_string(isolationIdx);
        ns = new NetworkNamespace(netnsName);
        ns->addCurrentThread();

        // Add this thread to the cgroup
        CGroup cgroup(BASE_CGROUP_NAME);
        cgroup.addCurrentThread();

        // Initialise wasm module
        module.initialise();

        // Add to unassigned set to request work
        redis->addToUnassignedSet(queueName);
    }

    Worker::~Worker() {
        delete ns;
    }

    void Worker::finish() {
        ns->removeCurrentThread();
        tokenPool.releaseToken(workerIdx);

        // Remove from sets to avoid more work
        if (functionSetName.empty()) {
            redis->removeFromUnassignedSet(queueName);
        } else {
            redis->srem(functionSetName, queueName);
        }

        // Final clean-up of wasm-related objects
        wasm::cleanUpWasmThread();
    }

    void Worker::finishCall(message::FunctionCall &call, const std::string &errorMsg) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->info("Finished {}", infra::funcToString(call));

        bool isSuccess = errorMsg.empty();
        if (!isSuccess) {
            call.set_outputdata(errorMsg);
        }

        // Set result
        redis->setFunctionResult(call, isSuccess);
    }

    void Worker::run() {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        // Wait for next function call on this thread's queue.
        while (true) {
            try {
                int timeout;
                if (functionSetName.empty()) {
                    timeout = UNBOUND_TIMEOUT;
                } else {
                    timeout = BOUND_TIMEOUT;
                }

                message::FunctionCall call = redis->nextFunctionCall(queueName, timeout);

                std::string errorMessage = this->executeCall(call);

                // Drop out if there's some issue
                if (!errorMessage.empty()) {
                    break;
                }
            }
            catch (infra::RedisNoResponseException &e) {
                // If exeucting a call, the thread will have been taken out of
                // sets waiting for work already. If it times out, it needs to remove
                // itself from whichever set it's in
                logger->debug("No calls made in timeout");

                this->finish();
                return;
            }
        }

        this->finish();
    }

    void Worker::runSingle() {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        message::FunctionCall call = redis->nextFunctionCall(queueName);
        std::string errorMessage = this->executeCall(call);

        // Drop out if there's some issue
        if (!errorMessage.empty()) {
            logger->error("Call failed with error: {}", errorMessage);
        }

        this->finish();
    }

    const std::string Worker::executeCall(message::FunctionCall &call) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        const std::chrono::steady_clock::time_point &t = prof::startTimer();

        // Bomb out if call isn't valid
        if (!infra::isValidFunction(call)) {
            std::string errorMessage = infra::funcToString(call) + " is not a valid function";

            this->finishCall(call, errorMessage);
            return errorMessage;
        }

        logger->info("Starting {}", infra::funcToString(call));

        // Create and execute the module
        wasm::CallChain callChain(call);
        try {
            module.execute(call, callChain);
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

        logger->debug("Adding worker to function set for {}", infra::funcToString(call));
        functionSetName = redis->addToFunctionSet(call, queueName);

        prof::logEndTimer("func-total", t);
        return empty;
    }
}
