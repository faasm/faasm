#define CATCH_CONFIG_RUNNER

// Disable catch signal catching to avoid interfering with dirty tracking
#define CATCH_CONFIG_NO_POSIX_SIGNALS 1

#include <catch2/catch.hpp>

#include "faabric_utils.h"
#include "utils.h"

#include <faabric/util/crash.h>
#include <faabric/util/logging.h>
#include <faaslet/Faaslet.h>
#include <storage/S3Wrapper.h>
#include <storage/SharedFiles.h>

FAABRIC_CATCH_LOGGER

int main(int argc, char* argv[])
{
    faabric::util::setUpCrashHandler();

    faabric::util::initLogging();
    storage::initFaasmS3();

    // Set Faaslets as the executors
    auto fac = std::make_shared<faaslet::FaasletFactory>();
    faabric::executor::setExecutorFactory(fac);

    int result = Catch::Session().run(argc, argv);

    fflush(stdout);
    storage::shutdownFaasmS3();

    return result;
}
