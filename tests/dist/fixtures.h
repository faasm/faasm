#pragma once

#include <catch2/catch.hpp>

#include "faabric_utils.h"

#include <codegen/MachineCodeGenerator.h>
#include <conf/FaasmConfig.h>
#include <faaslet/Faaslet.h>
#include <storage/FileLoader.h>

#include <faabric/scheduler/Scheduler.h>
#include <faabric/util/func.h>
#include <faabric/util/network.h>

namespace tests {

class DistTestsFixture
{
  public:
    DistTestsFixture()
      : redis(faabric::redis::Redis::getQueue())
      , sch(faabric::scheduler::getScheduler())
      , conf(faabric::util::getSystemConfig())
      , faasmConf(conf::getFaasmConfig())
    {
        redis.flushAll();

        // Planner reset
        sch.getPlannerClient()->ping();
        flushExecutors();
        resetPlanner();

        // Clean up the scheduler and make sure this host is available
        sch.shutdown();

        // Set slots
        setLocalRemoteSlots(faabric::util::getUsableCores(),
                            faabric::util::getUsableCores());

        // Set up executor
        std::shared_ptr<faaslet::FaasletFactory> fac =
          std::make_shared<faaslet::FaasletFactory>();
        faabric::scheduler::setExecutorFactory(fac);
    }

    ~DistTestsFixture()
    {
        // Planner reset
        flushExecutors();
        resetPlanner();

        conf.reset();
        faasmConf.reset();
    }

    void setLocalRemoteSlots(int nLocalSlots,
                             int nRemoteSlots,
                             int nLocalUsedSlots = 0,
                             int nRemoteUsedSlots = 0)
    {
        // If both hosts have the same number of available slots (usual in the
        // tests) we give the local host a higher total number of slots so that
        // it takes preference during scheduling. Otherwise, the planner will
        // sort hosts alphabetically in case of a tie, causing inconsistencies
        // in scheduling results
        bool sameSlotsAndAvailableSlots =
          (nLocalSlots - nLocalUsedSlots) == (nRemoteSlots - nRemoteUsedSlots);

        auto localResources = std::make_shared<faabric::HostResources>();
        if (sameSlotsAndAvailableSlots) {
            // Add more slots in total to break the tie
            localResources->set_slots(nLocalSlots + nLocalSlots);
            localResources->set_usedslots(nLocalSlots + nLocalUsedSlots);
        } else {
            localResources->set_slots(nLocalSlots);
            localResources->set_usedslots(nLocalUsedSlots);
        }
        sch.addHostToGlobalSet(getDistTestMasterIp(), localResources);

        auto remoteResources = std::make_shared<faabric::HostResources>();
        remoteResources->set_slots(nRemoteSlots);
        remoteResources->set_usedslots(nRemoteUsedSlots);
        sch.addHostToGlobalSet(getDistTestWorkerIp(), remoteResources);
    }

    std::string getDistTestMasterIp() { return conf.endpointHost; }

    std::string getDistTestWorkerIp()
    {
        if (workerIp.empty()) {
            workerIp = faabric::util::getIPFromHostname("dist-test-server");
        }
        return workerIp;
    }

  protected:
    faabric::redis::Redis& redis;
    faabric::scheduler::Scheduler& sch;
    faabric::util::SystemConfig& conf;
    conf::FaasmConfig& faasmConf;

    int functionCallTimeout = 60000;

    std::string masterIp;
    std::string workerIp;
};

class MpiDistTestsFixture : public DistTestsFixture
{
  public:
    void checkMpiAllocationAndResult(
      const std::shared_ptr<faabric::BatchExecuteRequest>& req,
      const std::vector<std::string>& expectedHosts)
    {
        auto& sch = faabric::scheduler::getScheduler();

        int nRetries = 3;
        int worldSize = req->messages(0).mpiworldsize();
        auto reqResp = sch.getBatchResult(req, 10000, worldSize);
        while (!reqResp && nRetries > 0) {
            --nRetries;
            SLEEP_MS(2000);
            reqResp = sch.getBatchResult(req, 10000, worldSize);
        }

        REQUIRE(reqResp != nullptr);
        std::vector<std::string> actualHosts;
        for (const auto& msg : reqResp->messages()) {
            REQUIRE(msg.returnvalue() == 0);
            actualHosts.push_back(msg.executedhost());
        }

        REQUIRE(actualHosts == expectedHosts);
    }

    void waitForMpiMessagesInFlight(
      const std::shared_ptr<faabric::BatchExecuteRequest>& req)
    {
        int worldSize = req->messages(0).mpiworldsize();

        SLEEP_MS(1000);
        auto batchMessages = sch.getPlannerClient()->getBatchMessages(req);
        while (batchMessages->messages_size() != worldSize) {
            SPDLOG_DEBUG("Waiting for MPI calls to be in-flight... ({} != {})",
                         batchMessages->messages_size(),
                         worldSize);
            SLEEP_MS(1000);
            batchMessages = sch.getPlannerClient()->getBatchMessages(req);
        }
    }

    // TODO: remove me
    void checkSchedulingFromExecGraph(
      const faabric::scheduler::ExecGraph& execGraph,
      const std::vector<std::string> expectedHosts)
    {
        std::vector<std::string> hostForRank =
          faabric::scheduler::getMpiRankHostsFromExecGraph(execGraph);
        REQUIRE(expectedHosts == hostForRank);
    }

    // TODO: remove me
    void checkSchedulingFromExecGraph(
      const faabric::scheduler::ExecGraph& execGraph,
      const std::vector<std::string> expectedHostsBefore,
      const std::vector<std::string> expectedHostsAfter)
    {
        std::vector<std::string> actualHostsBefore(
          execGraph.rootNode.msg.mpiworldsize());
        std::vector<std::string> actualHostsAfter(
          execGraph.rootNode.msg.mpiworldsize());

        auto actualHostsBeforeAndAfter =
          faabric::scheduler::getMigratedMpiRankHostsFromExecGraph(execGraph);

        REQUIRE(actualHostsBeforeAndAfter.first == expectedHostsBefore);
        REQUIRE(actualHostsBeforeAndAfter.second == expectedHostsAfter);
    }
};

}
