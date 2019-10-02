#include "IBMEndpoint.h"

#include <util/logging.h>
#include <worker/WorkerMain.h>

using namespace ibm;

int main() {
    util::initLogging ();
    const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

    worker::WorkerMain w;
    w.startBackground();

    logger->info("Starting IBM endpoint");
    IBMEndpoint endpoint;
    endpoint.start();

    logger->info("Shutting down ibm endpoint");
    w.shutdown();

    return 0;
}
