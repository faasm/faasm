#pragma once

#include "WorkerThreadPool.h"

#include <util/config.h>

namespace worker {
    class WorkerMain {
    public:
        WorkerMain();

        void startBackground();

        void awaitGlobalQueue();

        void shutdown();

        void setShareWork(bool in);

        void setSyncState(bool in);

    private:
        bool shareWork = true;
        bool syncState = true;

        util::SystemConfig &conf;
        worker::WorkerThreadPool pool;
        scheduler::Scheduler &scheduler;

    };
}