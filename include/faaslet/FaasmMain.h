#pragma once

#include "FaasletPool.h"

#include <faabric/state/StateServer.h>
#include <wasm/WasmModule.h>

#include <faabric/util/config.h>

namespace faaslet {
    class FaasmMain {
    public:
        FaasmMain();

        void startBackground();

        void awaitGlobalQueue();

        void shutdown();
    private:
        faabric::utilSystemConfig &conf;
        faaslet::FaasletPool pool;
        scheduler::Scheduler &scheduler;
    };
}