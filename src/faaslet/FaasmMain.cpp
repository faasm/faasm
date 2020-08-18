#include "FaasmMain.h"

#include <util/config.h>
#include <util/logging.h>
#include <module_cache/WasmModuleCache.h>

namespace faaslet {
    FaasmMain::FaasmMain() : conf(util::getSystemConfig()),
                               pool(conf.maxFaaslets),
                               scheduler(scheduler::getScheduler()) {

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

        // Start MPI thread in background
        pool.startMpiThread();
    }

    void FaasmMain::shutdown() {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->info("Removing from global working set");

        scheduler.clear();

        pool.shutdown();
    }
}