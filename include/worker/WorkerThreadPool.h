#pragma once

#include <scheduler/Scheduler.h>
#include <util/queue.h>

namespace worker {
    class WorkerThreadPool {
    public:
        explicit WorkerThreadPool(int nThreads);

        void startStateThread();

        void startGlobalQueueThread(bool join, bool dropOut);

        void startSharingThread();

        void startThreadPool(bool join);

        void reset();

        int getThreadToken();

        int getThreadCount();
    private:
        scheduler::Scheduler &scheduler;
        util::TokenPool threadTokenPool;

        std::thread stateThread;
        std::thread globalQueueThread;
        std::thread sharingQueueThread;
        std::thread poolThread;
    };
}