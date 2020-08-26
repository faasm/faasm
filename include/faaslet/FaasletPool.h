#pragma once

#include <faabric/scheduler/Scheduler.h>
#include <faabric/state/StateServer.h>
#include <faabric/util/queue.h>
#include <faabric/scheduler/FunctionCallServer.h>

namespace faaslet {
    class FaasletPool {
    public:
        explicit FaasletPool(int nThreads);

        void startFunctionCallServer();

        void startThreadPool();

        void startStateServer();

        void reset();

        int getThreadToken();

        int getThreadCount();

        bool isShutdown();

        void shutdown();
    private:
        std::atomic<bool> _shutdown;
        faabric::scheduler::Scheduler &scheduler;
        faabric::util::TokenPool threadTokenPool;
        faabric::state::StateServer stateServer;
        faabric::scheduler::FunctionCallServer functionServer;

        std::thread mpiThread;
        std::thread poolThread;
        std::vector<std::thread> poolThreads;
    };
}