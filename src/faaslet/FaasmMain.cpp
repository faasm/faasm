#include "FaasmMain.h"

#include <util/config.h>
#include <util/logging.h>
#include <module_cache/WasmModuleCache.h>

namespace faaslet {
    void flushFaasletHost() {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->warn("Flushing host {}", util::getSystemConfig().endpointHost);

        // Clear out any cached state
        state::getGlobalState().forceClearAll(false);

        // Clear shared files
        storage::FileSystem::clearSharedFiles();

        // Reset scheduler
        scheduler::Scheduler &sch = scheduler::getScheduler();
        sch.clear();
        sch.addHostToGlobalSet();

        // Clear out global message bus
        scheduler::getGlobalMessageBus().clear();

        // Clear zygotes
        module_cache::getWasmModuleCache().clear();
    }

    FaasmMain::FaasmMain() : conf(util::getSystemConfig()),
                               pool(conf.threadsPerWorker),
                               scheduler(scheduler::getScheduler()),
                               stateServer(state::getGlobalState()) {

    }

    void FaasmMain::startBackground() {
        scheduler.addHostToGlobalSet();

        conf.print();

        // Start thread pool in background
        pool.startThreadPool();

        // In-memory state
//        pool.startStateServer();

        // Work sharing
        pool.startSharingThread();

        // Start MPI thread in background
        pool.startMpiThread();
    }

    void FaasmMain::awaitGlobalQueue() {
        // Global queue listener (blocks until message received or timeout)
        pool.startGlobalQueueThread();
    }

    void FaasmMain::shutdown() {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->info("Removing from global working set");

        scheduler.clear();

        pool.shutdown();
    }
}