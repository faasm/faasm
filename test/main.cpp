#define CATCH_CONFIG_RUNNER

#include <catch/catch.hpp>
#include <util/logging.h>
#include <util/config.h>

int main(int argc, char *argv[]) {
    util::initLogging();

    // Make sure we're using redis
    util::SystemConfig conf = util::getSystemConfig();
    conf.globalMessageBus = "redis";
    conf.functionStorage = "local";

    int result = Catch::Session().run(argc, argv);

    return result;
}