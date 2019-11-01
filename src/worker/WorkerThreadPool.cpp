#include <worker/WorkerThread.h>

#include "WorkerThreadPool.h"

#include <scheduler/GlobalMessageBus.h>
#include <scheduler/SharingMessageBus.h>

namespace worker {

    WorkerThreadPool::WorkerThreadPool(int nThreads) :
            _shutdown(false),
            scheduler(scheduler::getScheduler()),
            threadTokenPool(nThreads) {

        // Ensure we can ping both redis instances
        redis::Redis::getQueue().ping();
        redis::Redis::getState().ping();
    }

    void WorkerThreadPool::startGlobalQueueThread() {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        util::SystemConfig &conf = util::getSystemConfig();

        logger->info("Starting global queue listener on {}", conf.queueName);

        globalQueueThread = std::thread([this, &conf, &logger] {
            scheduler::GlobalMessageBus &bus = scheduler::getGlobalMessageBus();
            scheduler::Scheduler &sch = scheduler::getScheduler();

            while (!this->isShutdown()) {
                try {
                    message::Message msg = bus.nextMessage(conf.globalMessageTimeout);

                    logger->debug("Got invocation for {} on {}", util::funcToString(msg, true), conf.queueName);
                    sch.callFunction(msg);
                }
                catch (scheduler::GlobalMessageBusNoMessageException &ex) {
                    logger->info("No message from global bus in {}ms, dropping out", conf.globalMessageTimeout);
                    return;
                }
            }

            // Will die gracefully at this point
        });

        // Waits for the queue to time out
        globalQueueThread.join();
    }

    void WorkerThreadPool::startSharingThread() {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->info("Starting work sharing listener");

        sharingQueueThread = std::thread([this] {
            scheduler::SharingMessageBus &sharingBus = scheduler::SharingMessageBus::getInstance();
            scheduler::Scheduler &sch = scheduler::getScheduler();

            const std::string nodeId = util::getNodeId();

            while (!this->isShutdown()) {
                const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
                try {
                    // This calls the scheduler, which will always attempt
                    // to execute locally. However, if not possible, this will
                    // again share the message, increasing the hops
                    message::Message msg = sharingBus.nextMessageForThisNode();

                    const std::string funcStr = util::funcToString(msg, true);
                    logger->debug("{} received shared call {} (scheduled for {})", nodeId, funcStr,
                                  msg.schedulednode());

                    sch.callFunction(msg);
                }
                catch (redis::RedisNoResponseException &ex) {
                    continue;
                }
            }

            // Will die gracefully at this point
        });
    }

    void WorkerThreadPool::startThreadPool() {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->info("Starting worker thread pool");

        // Spawn worker threads until we've hit the worker limit, thus creating a pool
        // that will replenish when one releases its token
        poolThread = std::thread([this] {
            const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

            while (!this->isShutdown()) {
                // Try to get an available slot (blocks if none available)
                int threadIdx = this->getThreadToken();

                // Double check shutdown condition
                if (this->isShutdown()) {
                    break;
                }

                // Spawn thread to execute function
                poolThreads.emplace_back(std::thread([this, threadIdx] {
                    WorkerThread w(threadIdx);

                    // Worker will now run for a long time
                    w.run();

                    // Handle thread finishing
                    threadTokenPool.releaseToken(w.threadIdx);
                }));
            }

            // Once shut down, wait for everything to die
            logger->info("Waiting for {} worker threads", poolThreads.size());
            for (auto &t : poolThreads) {
                if (t.joinable()) {
                    t.join();
                }
            }

            // Will die gracefully at this point
        });
    }

    void WorkerThreadPool::reset() {
        threadTokenPool.reset();
    }

    int WorkerThreadPool::getThreadToken() {
        return threadTokenPool.getToken();
    }

    int WorkerThreadPool::getThreadCount() {
        return threadTokenPool.taken();
    }

    bool WorkerThreadPool::isShutdown() {
        return _shutdown;
    }

    void WorkerThreadPool::shutdown() {
        _shutdown = true;

        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        if (globalQueueThread.joinable()) {
            logger->info("Waiting for global queue thread to finish");
            globalQueueThread.join();
        }

        if (stateThread.joinable()) {
            logger->info("Waiting for state thread to finish");
            stateThread.join();
        }

        if (sharingQueueThread.joinable()) {
            logger->info("Waiting for sharing queue thread to finish");
            sharingQueueThread.join();
        }

        if (poolThread.joinable()) {
            logger->info("Waiting for pool to finish");
            poolThread.join();
        }

        logger->info("Worker pool successfully shut down");
    }
}
