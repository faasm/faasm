#define CATCH_CONFIG_RUNNER

#include <catch/catch.hpp>
#include <faabric/util/logging.h>
#include "faabric_utils.h"

int main(int argc, char *argv[]) {
    faabric::utilinitLogging();

    int result = Catch::Session().run(argc, argv);

    fflush(stdout);

    return result;
}