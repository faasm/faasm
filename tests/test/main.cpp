#define CATCH_CONFIG_RUNNER

#include "faabric_utils.h"
#include "utils.h"

#include <catch2/catch.hpp>
#include <faabric/transport/context.h>
#include <faabric/util/logging.h>

#include <storage/S3Wrapper.h>

FAABRIC_CATCH_LOGGER

int main(int argc, char* argv[])
{
    storage::initSDK();
    faabric::transport::initGlobalMessageContext();
    faabric::util::initLogging();

    tests::cleanSystem();

    int result = Catch::Session().run(argc, argv);

    fflush(stdout);
    faabric::transport::closeGlobalMessageContext();

    storage::cleanUpSDK();

    return result;
}
