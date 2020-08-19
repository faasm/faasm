#pragma once

#include <scheduler/Scheduler.h>
#include <state/StateServer.h>
#include <util/queue.h>
#include <scheduler/FunctionCallServer.h>

namespace faaslet {
    class FaasletPool {
    public:
        explicit FaasletPool(int nThreads);

        void startFunctionCallServer();

        void startMpiThread();

        void startThreadPool();

        void startStateServer();

        void reset();

        int getThreadToken();

        int getThreadCount();

        bool isShutdown();

        void shutdown();
    private:
        std::atomic<bool> _shutdown;
        scheduler::Scheduler &scheduler;
        util::TokenPool threadTokenPool;
        state::StateServer stateServer;
        scheduler::FunctionCallServer functionServer;

        std::thread mpiThread;
        std::thread poolThread;
        std::vector<std::thread> poolThreads;
    };
}