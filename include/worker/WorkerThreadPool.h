#pragma once

#include <scheduler/Scheduler.h>
#include <util/queue.h>

namespace worker {
    class WorkerThreadPool {
    public:
        explicit WorkerThreadPool(int nThreads);

        void startStateThread();

        void startGlobalQueueThread();

        void startSharingThread();

        void startThreadPool(bool detach);

        void reset();

        int getThreadToken();

        int getThreadCount();
    private:
        scheduler::Scheduler &scheduler;
        util::TokenPool threadTokenPool;
    };
}