#include "WorkerThreadPool.h"

#include <aws/aws.h>

#include <util/logging.h>
#include <util/config.h>

int main() {
    util::initLogging();

    // Set up scheduler and add node to global set
    scheduler::Scheduler &sch = scheduler::getScheduler();
    sch.addNodeToGlobalSet();

    util::SystemConfig &config = util::getSystemConfig();
    config.print();

    // Initialise AWS (ignored if not running in AWS)
    awswrapper::initSDK();

    worker::WorkerThreadPool pool(config.threadsPerWorker);

    // Worker pool in background
    pool.startThreadPool();

    // Work sharing thread
    pool.startSharingThread();

    // State management thread
    pool.startStateThread();

    // Global queue listener (blocks until message received or timeout)
    pool.startGlobalQueueThread();

    const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
    logger->info("Removing from global working set");
    
    sch.clear();

    pool.shutdown();
}
