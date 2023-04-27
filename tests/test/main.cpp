#define CATCH_CONFIG_RUNNER

// Disable catch signal catching to avoid interfering with dirty tracking
#define CATCH_CONFIG_NO_POSIX_SIGNALS 1

#include "faabric_utils.h"
#include "utils.h"

#include <catch2/catch.hpp>

#include <faabric/util/crash.h>
#include <faabric/util/logging.h>

#include <storage/S3Wrapper.h>
#include <storage/SharedFiles.h>

FAABRIC_CATCH_LOGGER

int main(int argc, char* argv[])
{
    faabric::util::setUpCrashHandler();

    faabric::util::initLogging();
    storage::initFaasmS3();

    // Faabric stuff
    // TODO: this should happen in test fixtures
    // tests::cleanFaabric();

    // Clear local cache of shared files
    storage::SharedFiles::clear();

    // Set Faaslets as the executors
    std::shared_ptr<faabric::scheduler::ExecutorFactory> fac =
      std::make_shared<faaslet::FaasletFactory>();
    faabric::scheduler::setExecutorFactory(fac);

    int result = Catch::Session().run(argc, argv);

    fflush(stdout);
    storage::shutdownFaasmS3();

    return result;
}
