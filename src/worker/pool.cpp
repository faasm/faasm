#include "worker.h"

#include <infra/infra.h>
#include <prof/prof.h>

#include <spdlog/spdlog.h>
#include <thread>


namespace worker {
    // TODO - must match the underlying number of available namespaces. Good to decouple?
    static int N_THREADS = 40;

    static int UNBOUND_TIMEOUT = 180;
    static int BOUND_TIMEOUT = 30;

    static util::TokenPool tokenPool(N_THREADS);

    void startWorkerPool() {
        // Spawn worker threads until we've hit the limit (thus creating a pool that will replenish
        // when one releases its token)
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
        id = hostname + "_" + std::to_string(workerIdx);

        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->debug("Starting worker {}", workerIdx);

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
        module = new wasm::WasmModule();
        module->initialise();

        // TODO SCH work out whether to add to prewarm/ cold
        // Work out which queue and set
        currentQueue = infra::PREWARM_QUEUE;
        currentSet = infra::PREWARM_SET;
        redis->sadd(currentSet, id);
    }

    Worker::~Worker() {
        delete ns;

        delete module;
    }

    void Worker::finish() {
        ns->removeCurrentThread();
        tokenPool.releaseToken(workerIdx);

        // Remove from set
        redis->srem(currentSet, id);
    }

    void Worker::finishCall(message::Message &call, const std::string &errorMsg) {
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

    void Worker::run() {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        // Wait for next message
        while (true) {
            try {
                // Work out which timeout
                int timeout;
                if (currentQueue == infra::COLD_QUEUE || currentQueue == infra::PREWARM_QUEUE) {
                    timeout = UNBOUND_TIMEOUT;
                } else {
                    timeout = BOUND_TIMEOUT;
                }

                // Wait for next message
                message::Message msg = redis->nextMessage(currentQueue, timeout);

                // Handle the message
                std::string errorMessage;
                if (msg.type() == message::Message_MessageType_BIND) {
                    // Bind this worker to the given function
                    currentQueue = infra::getFunctionQueueName(msg);
                    currentSet = infra::getFunctionSetName(msg);

                    module->bindToFunction(msg);
                } else {
                    errorMessage = this->executeCall(msg);
                }

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

    void Worker::runSingle() {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        message::Message call = redis->nextMessage(currentQueue);
        std::string errorMessage = this->executeCall(call);

        // Drop out if there's some issue
        if (!errorMessage.empty()) {
            logger->error("Call failed with error: {}", errorMessage);
        }

        this->finish();
    }

    const std::string Worker::executeCall(message::Message &call) {
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
