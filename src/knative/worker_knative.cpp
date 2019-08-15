#include "KnativeEndpoint.h"

#include <util/logging.h>
#include <worker/WorkerMain.h>

using namespace knative;

int main() {
    util::initLogging();

    // Start the worker
    worker::WorkerMain w;
    w.startBackground();

    // Start endpoint (listening for requests)
    KnativeEndpoint endpoint;
    endpoint.start();

    w.shutdown();

    return 0;
}
