#include <worker/WorkerThread.h>

#include "WorkerThreadPool.h"
#include "DispatcherThread.h"
#include "StateThread.h"

#include <scheduler/GlobalMessageQueue.h>

namespace worker {

    WorkerThreadPool::WorkerThreadPool(int nThreads) :
            scheduler(scheduler::getScheduler()),
            threadTokenPool(nThreads) {

        // Ensure we can ping both redis instances
        redis::Redis::getQueue().ping();
        redis::Redis::getState().ping();
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

        // Thread to listen for global incoming messages
        std::thread globalDispatchThread([this] {
            DispatcherThread t(scheduler.getGlobalQueue());
            t.run();
        });
        globalDispatchThread.detach();

        // Thread to listen for incoming sharing messages
        std::thread sharingDispatchThread([this] {
            DispatcherThread t(scheduler.getHostSharingQueue());
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
