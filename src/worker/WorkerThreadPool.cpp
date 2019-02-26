#include <worker/WorkerThread.h>

#include "WorkerThreadPool.h"
#include "StateThread.h"

#include <scheduler/GlobalMessageBus.h>
#include <scheduler/SharingMessageBus.h>

namespace worker {

    WorkerThreadPool::WorkerThreadPool(int nThreads) :
            scheduler(scheduler::getScheduler()),
            threadTokenPool(nThreads) {

        // Ensure we can ping both redis instances
        redis::Redis::getQueue().ping();
        redis::Redis::getState().ping();
    }

    void WorkerThreadPool::startStateThread() {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        util::SystemConfig &conf = util::getSystemConfig();

        if (!conf.fullAsync) {
            logger->info("Starting state sync thread");

            std::thread stateThread([] {
                StateThread s;
                s.run();
            });
            stateThread.detach();
        }
    }

    void WorkerThreadPool::startGlobalQueueThread(bool detach, bool dropOut) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        util::SystemConfig &conf = util::getSystemConfig();

        logger->info("Starting global queue listener on {}", conf.queueName);

        std::thread globalDispatchThread([&conf, &logger, &dropOut] {
            scheduler::GlobalMessageBus &bus = scheduler::getGlobalMessageBus();
            scheduler::Scheduler &sch = scheduler::getScheduler();

            while (true) {
                try {
                    message::Message msg = bus.nextMessage(conf.unboundTimeout);

                    logger->debug("Got invocation for {} on {}", util::funcToString(msg), conf.queueName);
                    sch.callFunction(msg);
                }
                catch (scheduler::GlobalMessageBusNoMessageException &ex) {
                    if(dropOut) {
                        logger->info("No message from global bus in {}ms, dropping out", conf.unboundTimeout);
                        return;
                    }
                    else {
                        continue;
                    }
                }
            }
        });

        if(detach) {
            globalDispatchThread.detach();
        }
        else {
            globalDispatchThread.join();
        }
    }

    void WorkerThreadPool::startSharingThread() {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->info("Starting work sharing listener");

        std::thread sharingDispatchThread([] {
            scheduler::SharingMessageBus &sharingBus = scheduler::SharingMessageBus::getInstance();
            scheduler::Scheduler &sch = scheduler::getScheduler();

            while (true) {
                try {
                    message::Message msg = sharingBus.nextMessageForThisHost();
                    sch.callFunction(msg);
                }
                catch (redis::RedisNoResponseException &ex) {
                    continue;
                }
            }
        });
        sharingDispatchThread.detach();
    }

    void WorkerThreadPool::startThreadPool(bool detach) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        // Spawn worker threads until we've hit the worker limit, thus creating a pool
        // that will replenish when one releases its token
        std::thread poolThread([this] {
            while (true) {
                // Try to get an available slot (blocks if none available)
                int threadIdx = this->getThreadToken();

                // Spawn thread to execute function
                std::thread funcThread([this, threadIdx] {
                    WorkerThread w(threadIdx);

                    // Worker will now run for a long time
                    w.run();

                    // Handle thread finishing
                    threadTokenPool.releaseToken(w.threadIdx);
                });

                funcThread.detach();
            }
        });

        if(detach) {
            logger->info("Starting pool with {} threads in background", threadTokenPool.size());
            poolThread.detach();
        } else {
            logger->info("Starting pool with {} threads in main thread", threadTokenPool.size());
            poolThread.join();
        }
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
}
