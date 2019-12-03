#include "WorkerMain.h"

#if AWS_SUPPORT == 1
#include <aws/aws.h>
#endif

#include <util/logging.h>

int main() {
    util::initLogging();

#if AWS_SUPPORT == 1
    // Initialise AWS (ignored if not running in AWS)
    awswrapper::initSDK();
#endif
    
    // Start the worker
    worker::WorkerMain w;
    w.startBackground();

    // Wait on global queue
    w.awaitGlobalQueue();

    // Stop
    w.shutdown();
}
