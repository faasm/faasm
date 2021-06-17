#define CATCH_CONFIG_RUNNER

#include "utils.h"
#include <catch2/catch.hpp>
#include <faabric/util/logging.h>

struct LogListener : Catch::TestEventListenerBase
{
    // Note, we must call base class versions of overridden methods
    // https://github.com/catchorg/Catch2/pull/1617
    using TestEventListenerBase::TestEventListenerBase;

    void testCaseStarting(Catch::TestCaseInfo const& testInfo) override
    {
        this->Catch::TestEventListenerBase::testCaseStarting(testInfo);

        SPDLOG_INFO("=============================================");
        SPDLOG_INFO("TEST: {}", testInfo.name);
        SPDLOG_INFO("=============================================");
    }

    void sectionStarting(Catch::SectionInfo const& sectionInfo) override
    {
        this->Catch::TestEventListenerBase::sectionStarting(sectionInfo);

        // Tests without any sections will be default have one section with the
        // same name as the test
        if (sectionInfo.name != currentTestCaseInfo->name) {
            SPDLOG_INFO("---------------------------------------------");
            SPDLOG_INFO("SECTION: {}", sectionInfo.name);
            SPDLOG_INFO("---------------------------------------------");
        }
    }
};

CATCH_REGISTER_LISTENER(LogListener)

int main(int argc, char* argv[])
{
    faabric::util::initLogging();

    // Clean the system
    tests::cleanSystem();

    int result = Catch::Session().run(argc, argv);

    fflush(stdout);

    return result;
}
