#define CATCH_CONFIG_RUNNER

#include <catch/catch.hpp>

#include <util/logging.h>
#include <util/config.h>
#include <curl/curl.h>

int main(int argc, char *argv[]) {
    util::SystemConfig &conf = util::getSystemConfig();
    conf.functionStorage = "ibm";

    util::initLogging();

    curl_global_init(CURL_GLOBAL_ALL);

    int result = Catch::Session().run(argc, argv);

    return result;
}