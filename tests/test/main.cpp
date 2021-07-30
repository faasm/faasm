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
    faabric::util::initLogging();
    storage::initFaasmS3();
    faabric::transport::initGlobalMessageContext();

    tests::cleanSystem();

    int result = Catch::Session().run(argc, argv);

    fflush(stdout);
    faabric::transport::closeGlobalMessageContext();

    storage::shutdownFaasmS3();

    return result;
}
