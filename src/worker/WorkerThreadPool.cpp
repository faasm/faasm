#include <worker/WorkerThread.h>

#include "WorkerThreadPool.h"
#include "DispatcherThread.h"
#include "StateThread.h"

#include <scheduler/GlobalMessageBus.h>

namespace worker {

    WorkerThreadPool::WorkerThreadPool(int nThreads) :
            scheduler(scheduler::getScheduler()),
            threadTokenPool(nThreads) {

        // Ensure we can ping both redis instances
        redis::Redis::getQueue().ping();
        redis::Redis::getState().ping();
    }

    void WorkerThreadPool::start() {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->info("Starting pool with {} threads", threadTokenPool.size());

        // Spawn the state thread first if not running in full async
        util::SystemConfig &conf = util::getSystemConfig();
        if (!conf.fullAsync) {
            logger->info("Starting state sync thread");

            std::thread stateThread([] {
                StateThread s;
                s.run();
            });
            stateThread.detach();
        }

        // Thread to listen for global incoming messages
        logger->info("Starting global queue listener (queue = {})", INCOMING_QUEUE);

        std::thread globalDispatchThread([] {
            DispatcherThread t(INCOMING_QUEUE);
            t.run();
        });
        globalDispatchThread.detach();

        // Thread to listen for incoming sharing messages
        const std::string sharingQueueName = scheduler.getHostSharingQueueName();
        logger->info("Starting work sharing listener (queue = {})", sharingQueueName);

        std::thread sharingDispatchThread([sharingQueueName] {
            DispatcherThread t(sharingQueueName);
            t.run();
        });
        sharingDispatchThread.detach();

        // Spawn worker threads until we've hit the worker limit, thus creating a pool
        // that will replenish when one releases its token
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
