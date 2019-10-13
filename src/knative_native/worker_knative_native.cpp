#include "KnativeNativeEndpoint.h"

#include <util/logging.h>

using namespace knative_native;

int main() {
    util::initLogging ();
    const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

    // Start endpoint (will also have multiple threads)
    logger->info("Starting knative native endpoint");
    KnativeNativeEndpoint endpoint;
    endpoint.start();

    logger->info("Shutting down knative native endpoint");

    return 0;
}
