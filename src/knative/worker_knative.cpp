#include "KnativeEndpoint.h"

#include <util/logging.h>
#include <util/config.h>
#include <worker/WorkerThreadPool.h>

using namespace knative;

int main() {
    util::initLogging();

    // Set up scheduler and add node to global set
    scheduler::Scheduler &sch = scheduler::getScheduler();
    sch.addNodeToGlobalSet();

    util::SystemConfig &config = util::getSystemConfig();
    config.print();

    worker::WorkerThreadPool pool(config.threadsPerWorker);

    // Worker pool in background
    pool.startThreadPool();

    // Work sharing thread
    pool.startSharingThread();

    // State management thread
    pool.startStateThread();

    // Start endpoint (listening for requests)
    KnativeEndpoint endpoint;
    endpoint.start();

    // Tidy up
    sch.clear();
    pool.shutdown();

    return 0;
}
