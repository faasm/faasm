#define CATCH_CONFIG_RUNNER

#include <catch/catch.hpp>
#include <util/logging.h>
#include "utils.h"

int main(int argc, char *argv[]) {
    util::initLogging();

    // Clean the system
    tests::cleanSystem();

    int result = Catch::Session().run(argc, argv);

    fflush(stdout);

    return result;
}