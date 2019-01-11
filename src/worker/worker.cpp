#include "worker.h"

int main() {
    util::initLogging();

    util::SystemConfig &config = util::getSystemConfig();
    config.print();

    worker::WorkerThreadPool pool(config.threadsPerWorker, config.prewarmTarget);
    pool.start();
}
