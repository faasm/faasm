#include "worker.h"

int main() {
    util::initLogging();
    worker::startWorkerThreadPool();
}
