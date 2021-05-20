#include <catch2/catch.hpp>

#include "fixtures.h"

#include <faabric/scheduler/Scheduler.h>

namespace tests {
TEST_CASE_METHOD(DistTestsFixture, "Test available hosts", "[scheduler]")
{
    auto& sch = faabric::scheduler::getScheduler();
    sch.addHostToGlobalSet();

    std::set<std::string> actual = sch.getAvailableHosts();

    std::set<std::string> expected = { MASTER_IP, WORKER_IP };
    REQUIRE(actual == expected);
}
}
