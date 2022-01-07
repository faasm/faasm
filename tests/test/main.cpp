#define CATCH_CONFIG_RUNNER

#include "faabric_utils.h"
#include "utils.h"

#include <catch2/catch.hpp>

#include <faabric/transport/context.h>
#include <faabric/util/crash.h>
#include <faabric/util/logging.h>

#include <storage/S3Wrapper.h>
#include <system/memory.h>

// Disable catch signal handling to avoid interfering with dirty tracking
#define CATCH_CONFIG_NO_POSIX_SIGNALS 1

FAABRIC_CATCH_LOGGER

int main(int argc, char* argv[])
{
    isolation::checkStackSize();

    faabric::util::setUpCrashHandler();

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
