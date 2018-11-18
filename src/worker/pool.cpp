#include "worker.h"

#include <infra/infra.h>
#include <prof/prof.h>

#include <spdlog/spdlog.h>
#include <thread>


namespace worker {
    // TODO - must match the underlying number of available namespaces. Good to decouple?
    static int N_THREADS = 10;

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
    }

    void Worker::finishCall(const std::string &errorMsg) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->info("Finished ({}/{})", call.user(), call.function());

        bool isSuccess = errorMsg.empty();
        if (!isSuccess) {
            call.set_outputdata(errorMsg);
        }

        // Set result
        redis->setFunctionResult(call, isSuccess);
    }

    void Worker::run() {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        // Wait for next function call on this thread's queue
        try {
            call = redis->nextFunctionCall(queueName);
        }
        catch (infra::RedisNoResponseException &e) {
            logger->debug("No calls made in timeout");
            this->finish();
            return;
        }

        const std::chrono::steady_clock::time_point &t = prof::startTimer();

        // Bomb out if call isn't valid
        if (!infra::isValidFunction(call)) {
            std::string errorMessage = call.user();
            errorMessage.append(" - ");
            errorMessage.append(call.function());
            errorMessage.append(" is not a valid function");

            this->finish();
            this->finishCall(errorMessage);
            return;
        }

        logger->info("Starting ({}/{})", call.user(), call.function());

        // Create and execute the module
        wasm::CallChain callChain(call);
        try {
            module.execute(call, callChain);
        }
        catch (const std::exception &e) {
            std::string errorMessage = "Error: " + std::string(e.what());
            logger->error(errorMessage);

            this->finish();
            this->finishCall(errorMessage);
            return;
        }

        // Process any chained calls
        std::string chainErrorMessage = callChain.execute();
        if (!chainErrorMessage.empty()) {
            this->finish();
            this->finishCall(chainErrorMessage);
        }

        this->finish();
        this->finishCall("");

        prof::logEndTimer("func-total", t);
    }
}
