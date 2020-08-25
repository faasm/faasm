#include "FaasmMain.h"

#include <faabric/util/config.h>
#include <faabric/util/logging.h>
#include <module_cache/WasmModuleCache.h>

namespace faaslet {
    FaasmMain::FaasmMain() : conf(faabric::util::getSystemConfig()),
                               pool(conf.maxFaaslets),
                               scheduler(faabric::scheduler::getScheduler()) {

    }

    void FaasmMain::startBackground() {
        scheduler.addHostToGlobalSet();

        conf.print();

        // Start thread pool in background
        pool.startThreadPool();

        // In-memory state
        pool.startStateServer();

        // Work sharing
        pool.startFunctionCallServer();
    }

    void FaasmMain::shutdown() {
        const std::shared_ptr<spdlog::logger> &logger = faabric::util::getLogger();
        logger->info("Removing from global working set");

        scheduler.clear();

        pool.shutdown();
    }
}