#include "WorkerMain.h"

#include <util/config.h>
#include <util/logging.h>

namespace worker {
    WorkerMain::WorkerMain() : conf(util::getSystemConfig()),
                               pool(conf.threadsPerWorker),
                               scheduler(scheduler::getScheduler()) {

    }

    void WorkerMain::startBackground() {
        scheduler.addNodeToGlobalSet();

        conf.print();

        // Start thread pool in background
        pool.startThreadPool();

        // In-memory state
        pool.startStateServer();

        // Work sharing
        pool.startSharingThread();

        // Start MPI thread in background
//        pool.startMpiThread();
    }

    void WorkerMain::awaitGlobalQueue() {
        // Global queue listener (blocks until message received or timeout)
        pool.startGlobalQueueThread();
    }

    void WorkerMain::shutdown() {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->info("Removing from global working set");

        scheduler.clear();

        pool.shutdown();
    }
}