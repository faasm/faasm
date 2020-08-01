#pragma once

#include <scheduler/Scheduler.h>
#include <state/StateServer.h>
#include <util/queue.h>

namespace faaslet {
    class FaasletPool {
    public:
        explicit FaasletPool(int nThreads);

        void startGlobalQueueThread();

        void startSharingThread();

        void startMpiThread();

        void startThreadPool();

        void startStateServer();

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
        state::StateServer server;

        std::thread stateThread;
        std::thread globalQueueThread;
        std::thread sharingQueueThread;
        std::thread mpiThread;
        std::thread poolThread;
        std::vector<std::thread> poolThreads;
    };
}