#include "IBMEndpoint.h"

#include <util/logging.h>
#include <curl/curl.h>

using namespace ibm;

int main() {
    util::SystemConfig &conf = util::getSystemConfig();
    conf.logLevel = "debug";

    util::initLogging ();
    const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

    // IBM worker makes heavy use of cURL
    curl_global_init(CURL_GLOBAL_ALL);

    logger->info("Starting IBM endpoint");

    IBMEndpoint endpoint;
    endpoint.start();

    logger->info("Shutting down ibm endpoint");
    return 0;
}
