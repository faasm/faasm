#include "WorkerMain.h"

#include <aws/aws.h>

#include <util/logging.h>

int main() {
    util::initLogging();

    // Initialise AWS (ignored if not running in AWS)
    awswrapper::initSDK();

    // Start the worker
    worker::WorkerMain w;
    w.startBackground();

    // Wait on global queue
    w.awaitGlobalQueue();

    // Stop
    w.shutdown();
}
