#pragma once

#include "WorkerThreadPool.h"

#include <wasm/WasmModule.h>

#include <util/config.h>

namespace worker {
    class WorkerMain {
    public:
        WorkerMain();

        void startBackground();

        void awaitGlobalQueue();

        void shutdown();
    private:
        util::SystemConfig &conf;
        worker::WorkerThreadPool pool;
        scheduler::Scheduler &scheduler;
    };
}