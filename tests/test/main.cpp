#define CATCH_CONFIG_RUNNER

#include "utils.h"
#include <catch2/catch.hpp>
#include <faabric/util/logging.h>

int main(int argc, char* argv[])
{
    // Clean the system
    tests::cleanSystem();

    int result = Catch::Session().run(argc, argv);

    fflush(stdout);

    return result;
}
