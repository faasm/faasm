#pragma once

#include <scheduler/Scheduler.h>
#include <util/queue.h>

namespace worker {
    class WorkerThreadPool {
    public:
        explicit WorkerThreadPool(int nThreads);

        void startGlobalQueueThread();

        void startSharingThread();

        void startThreadPool();

        void reset();

        int getThreadToken();

        int getThreadCount();

        bool isShutdown();

        void shutdown();

        void preparePythonRuntime();
    private:
        std::atomic<bool> _shutdown;
        scheduler::Scheduler &scheduler;
        util::TokenPool threadTokenPool;

        std::thread stateThread;
        std::thread globalQueueThread;
        std::thread sharingQueueThread;
        std::thread poolThread;
        std::vector<std::thread> poolThreads;
    };
}