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

            stateThread = std::thread([] {
                StateThread s;
                s.run();
            });
        }
    }

    void WorkerThreadPool::startGlobalQueueThread(bool join, bool dropOut) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        util::SystemConfig &conf = util::getSystemConfig();

        logger->info("Starting global queue listener on {}", conf.queueName);

        globalQueueThread = std::thread ([&conf, &logger, &dropOut] {
            scheduler::GlobalMessageBus &bus = scheduler::getGlobalMessageBus();
            scheduler::Scheduler &sch = scheduler::getScheduler();

            while (true) {
                try {
                    message::Message msg = bus.nextMessage(conf.globalMessageTimeout);

                    logger->debug("Got invocation for {} on {}", util::funcToString(msg), conf.queueName);
                    sch.callFunction(msg);
                }
                catch (scheduler::GlobalMessageBusNoMessageException &ex) {
                    if(dropOut) {
                        logger->info("No message from global bus in {}s, dropping out", conf.globalMessageTimeout);
                        return;
                    }
                    else {
                        continue;
                    }
                }
            }
        });

        if(join) {
            globalQueueThread.join();
        }
    }

    void WorkerThreadPool::startSharingThread() {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->info("Starting work sharing listener");

        sharingQueueThread = std::thread([] {
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
    }

    void WorkerThreadPool::startThreadPool(bool join) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        // Spawn worker threads until we've hit the worker limit, thus creating a pool
        // that will replenish when one releases its token
        poolThread = std::thread([this] {
            // We need to keep these threads in scope until the program terminates
            std::vector<std::thread> threads;

            while (true) {
                // Try to get an available slot (blocks if none available)
                int threadIdx = this->getThreadToken();

                // Spawn thread to execute function
                threads.push_back(std::thread([this, threadIdx] {
                    WorkerThread w(threadIdx);

                    // Worker will now run for a long time
                    w.run();

                    // Handle thread finishing
                    threadTokenPool.releaseToken(w.threadIdx);
                }));
            }
        });

        if(join) {
            logger->info("Joining pool with {} threads in main thread", threadTokenPool.size());
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
