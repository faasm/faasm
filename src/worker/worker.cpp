#include "worker.h"

int main() {
    util::initLogging();
    worker::startWorkerPool();
}
