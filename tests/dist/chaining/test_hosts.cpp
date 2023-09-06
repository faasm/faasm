#include <catch2/catch.hpp>

#include "fixtures.h"

#include <faabric/scheduler/Scheduler.h>

namespace tests {
TEST_CASE_METHOD(DistTestsFixture, "Test available hosts", "[scheduler]")
{
    auto& sch = faabric::scheduler::getScheduler();
    sch.addHostToGlobalSet();

    // Check the available hosts
    auto availableHosts = plannerCli.getAvailableHosts();
    std::set<std::string> actual;
    for (auto& host : availableHosts) {
        actual.insert(host.ip());
    }

    std::set<std::string> expected = { getDistTestMasterIp(),
                                       getDistTestWorkerIp() };

    REQUIRE(actual == expected);
}
}
