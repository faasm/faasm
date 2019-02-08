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
    pool.start();
}
