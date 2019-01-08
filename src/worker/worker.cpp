#include "worker.h"

int main() {
    util::initLogging();

    util::SystemConfig &config = util::getSystemConfig();
    config.print();

    worker::WorkerThreadPool pool;
    pool.start();
}
