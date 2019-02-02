#pragma once

#include <util/queue.h>

namespace worker {
    class WorkerThreadPool {
    public:
        explicit WorkerThreadPool(int nThreads);

        void start();

        void reset();

        int getThreadToken();

        int getThreadCount();
    private:
        util::TokenPool threadTokenPool;
    };
}