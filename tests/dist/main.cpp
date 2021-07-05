#define CATCH_CONFIG_RUNNER

#include "faabric_utils.h"

#include <catch2/catch.hpp>

#include <faaslet/Faaslet.h>

#include <faabric/endpoint/FaabricEndpoint.h>
#include <faabric/runner/FaabricMain.h>
#include <faabric/scheduler/ExecutorFactory.h>
#include <faabric/transport/context.h>
#include <faabric/util/logging.h>

using namespace faabric::scheduler;

FAABRIC_CATCH_LOGGER

int main(int argc, char* argv[])
{
    faabric::transport::initGlobalMessageContext();
    faabric::util::initLogging();

    // Start everything up
    SPDLOG_INFO("Starting distributed test server on master");
    std::shared_ptr<faaslet::FaasletFactory> fac =
      std::make_shared<faaslet::FaasletFactory>();

    // WARNING: all 0MQ sockets have to have gone *out of scope* before we shut
    // down the context, therefore this segment must be in a nested scope (or
    // another function).
    int result;
    {
        faabric::runner::FaabricMain m(fac);
        m.startBackground();

        // Wait for things to start
        usleep(3000 * 1000);

        // Run the tests
        result = Catch::Session().run(argc, argv);
        fflush(stdout);

        // Shut down
        SPDLOG_INFO("Shutting down");
        m.shutdown();
    }

    faabric::transport::closeGlobalMessageContext();
    return result;
}
