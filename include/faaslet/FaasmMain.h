#pragma once

#include "FaasletPool.h"

#include <state/StateServer.h>
#include <wasm/WasmModule.h>

#include <util/config.h>

namespace faaslet {
    void flushFaasletHost();

    class FaasmMain {
    public:
        FaasmMain();

        void startBackground();

        void awaitGlobalQueue();

        void shutdown();
    private:
        util::SystemConfig &conf;
        faaslet::FaasletPool pool;
        scheduler::Scheduler &scheduler;
    };
}