#define CATCH_CONFIG_RUNNER

#include "utils.h"
#include <catch2/catch.hpp>
#include <faabric/util/logging.h>

struct LogListener : Catch::TestEventListenerBase
{
    using TestEventListenerBase::TestEventListenerBase;

    void testCaseStarting(Catch::TestCaseInfo const& testInfo) override
    {
        auto logger = faabric::util::getLogger();
        logger->info("---------------------------------------------");
        logger->info("TEST: {}", testInfo.name);
        logger->info("---------------------------------------------");
    }
};

CATCH_REGISTER_LISTENER(LogListener)

int main(int argc, char* argv[])
{
    // Clean the system
    tests::cleanSystem();

    int result = Catch::Session().run(argc, argv);

    fflush(stdout);

    return result;
}
