#include <catch2/catch.hpp>

#include "fixtures.h"

#include <faabric/scheduler/Scheduler.h>
#include <faabric/util/ExecGraph.h>
#include <faabric/util/batch.h>

#include <set>

namespace tests {

TEST_CASE_METHOD(MpiDistTestsFixture,
                 "Test running an MPI function spanning two hosts",
                 "[mpi]")
{
    // Set up resources so that application is split among two hosts (two ranks
    // locally, two ranks remotely)
    int worldSize = 4;
    setLocalRemoteSlots(worldSize, 2, worldSize - 2, 0);

    // Set up the message
    std::shared_ptr<faabric::BatchExecuteRequest> req =
      faabric::util::batchExecFactory("mpi", "mpi_bcast", 1);
    req->mutable_messages(0)->set_ismpi(true);
    req->mutable_messages(0)->set_mpiworldsize(worldSize);
    req->mutable_messages(0)->set_recordexecgraph(true);

    // Call the functions
    plannerCli.callFunctions(req);

    // Check it's successful
    auto result = getMpiBatchResult(req->messages(0));

    // Check exec graph
    auto execGraph = faabric::util::getFunctionExecGraph(result);
    int numNodes = faabric::util::countExecGraphNodes(execGraph);
    REQUIRE(numNodes == worldSize);
    std::set<std::string> hosts = faabric::util::getExecGraphHosts(execGraph);
    REQUIRE(hosts.size() == 2);
    std::vector<std::string> expectedHosts = { getDistTestMasterIp(),
                                               getDistTestMasterIp(),
                                               getDistTestWorkerIp(),
                                               getDistTestWorkerIp() };
    checkSchedulingFromExecGraph(execGraph, expectedHosts);
}
}
