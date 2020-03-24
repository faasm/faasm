#define CATCH_CONFIG_RUNNER

#include <catch/catch.hpp>

#include <util/logging.h>
#include <util/environment.h>
#include <curl/curl.h>
#include <util/config.h>

int main(int argc, char *argv[]) {
    util::initLogging();

    util::SystemConfig &conf = util::getSystemConfig();
    conf.hostType = "knative";

    // Set up the required environment variables
    util::setEnvVar("FAASM_INVOKE_HOST", "localhost");
    util::setEnvVar("FAASM_INVOKE_PORT", "8080");

    curl_global_init(CURL_GLOBAL_ALL);

    int result = Catch::Session().run(argc, argv);

    return result;
}