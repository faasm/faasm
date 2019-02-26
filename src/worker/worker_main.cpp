#include "WorkerThreadPool.h"

#include <aws/aws.h>

#include <util/logging.h>
#include <util/config.h>

int main() {
    util::initLogging();

    util::SystemConfig &config = util::getSystemConfig();
    config.print();

    awswrapper::initSDK();

    worker::WorkerThreadPool pool(config.threadsPerWorker);

    // Global queue listener
    pool.startGlobalQueueThread(true, false);

    // Work sharing thread
    pool.startSharingThread();

    // State management thread
    pool.startStateThread();

    // Worker pool run in main thread
    pool.startThreadPool(false);
}
