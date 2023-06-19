#include <catch2/catch.hpp>

#include "fixtures.h"

#include <faabric/scheduler/Scheduler.h>

namespace tests {

TEST_CASE_METHOD(MpiDistTestsFixture,
                 "Test running two MPI applications at the same time",
                 "[mpi]")
{
    // Set up this host's resources
    int nLocalSlots = 5;
    int worldSize = 4;
    faabric::HostResources res;
    res.set_usedslots(0);
    res.set_slots(nLocalSlots);
    sch.setThisHostResources(res);

    // Set up the message
    std::shared_ptr<faabric::BatchExecuteRequest> req =
      faabric::util::batchExecFactory("mpi", "mpi_long_alltoall", 1);
    faabric::Message& msg = req->mutable_messages()->at(0);
    msg.set_ismpi(true);
    msg.set_mpiworldsize(worldSize);
    msg.set_recordexecgraph(true);

    // Call the functions
    sch.callFunctions(req);

    // Set up the second message
    std::shared_ptr<faabric::BatchExecuteRequest> reqCopy =
      faabric::util::batchExecFactory("mpi", "mpi_long_alltoall", 1);
    faabric::Message& msgCopy = reqCopy->mutable_messages()->at(0);
    msgCopy.set_ismpi(true);
    msgCopy.set_mpiworldsize(worldSize);
    msgCopy.set_recordexecgraph(true);

    // Call the functions for a second time
    sch.callFunctions(reqCopy);

    // Check both results are successful
    faabric::Message result = sch.getFunctionResult(msg, functionCallTimeout);
    REQUIRE(result.returnvalue() == 0);
    faabric::Message resultCopy =
      sch.getFunctionResult(msgCopy, functionCallTimeout);
    REQUIRE(result.returnvalue() == 0);

    // Get the execution graph for both requests
    auto execGraph = sch.getFunctionExecGraph(result.id());
    auto execGraphCopy = sch.getFunctionExecGraph(resultCopy.id());

    // Builld the expectation for both requests
    std::vector<std::string> expectedHosts(worldSize, getDistTestMasterIp());
    std::vector<std::string> expectedHostsCopy = { getDistTestMasterIp(),
                                                   getDistTestWorkerIp(),
                                                   getDistTestWorkerIp(),
                                                   getDistTestWorkerIp() };

    // Check the expecation against the actual execution graphs
    checkSchedulingFromExecGraph(execGraph, expectedHosts);
    checkSchedulingFromExecGraph(execGraphCopy, expectedHostsCopy);
}
}
