#include "WorkerThreadPool.h"

#include <util/logging.h>
#include <util/config.h>

int main() {
    util::initLogging();

    util::SystemConfig &config = util::getSystemConfig();
    config.print();

    worker::WorkerThreadPool pool(config.threadsPerWorker);
    pool.start();
}
