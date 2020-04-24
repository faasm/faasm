#include "WorkerMain.h"

#include <util/logging.h>

int main() {
    util::initLogging();
    
    // Start the worker
    worker::WorkerMain w;
    w.startBackground();

    // Wait on global queue
    w.awaitGlobalQueue();

    // Stop
    w.shutdown();
}
