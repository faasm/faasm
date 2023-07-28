#pragma once

#include <catch2/catch.hpp>

#include "faabric_utils.h"

#include <codegen/MachineCodeGenerator.h>
#include <conf/FaasmConfig.h>
#include <faabric/planner/PlannerClient.h>
#include <faabric/scheduler/Scheduler.h>
#include <faabric/util/ExecGraph.h>
#include <faabric/util/func.h>
#include <faabric/util/network.h>
#include <faaslet/Faaslet.h>
#include <storage/FileLoader.h>

namespace tests {

class DistTestsFixture
{
  public:
    DistTestsFixture()
      : redis(faabric::redis::Redis::getQueue())
      , sch(faabric::scheduler::getScheduler())
      , plannerCli(faabric::planner::getPlannerClient())
      , conf(faabric::util::getSystemConfig())
      , faasmConf(conf::getFaasmConfig())
    {
        redis.flushAll();

        // Planner reset
        plannerCli.ping();
        resetPlanner();

        // Clean up the scheduler
        sch.reset();

        // Set slots
        setLocalRemoteSlots(faabric::util::getUsableCores(),
                            faabric::util::getUsableCores());

        // Set up executor
        std::shared_ptr<faaslet::FaasletFactory> fac =
          std::make_shared<faaslet::FaasletFactory>();
        faabric::scheduler::setExecutorFactory(fac);
    }

    std::string getDistTestMasterIp() { return conf.endpointHost; }

    std::string getDistTestWorkerIp()
    {
        if (workerIp.empty()) {
            workerIp = faabric::util::getIPFromHostname("dist-test-server");
        }
        return workerIp;
    }

    ~DistTestsFixture()
    {
        conf.reset();
        faasmConf.reset();
        // Flush before resetting, as otherwise the planner won't have any
        // recorded hosts
        flushPlannerWorkers();
        resetPlanner();
    }

    void setLocalRemoteSlots(int nLocalSlots,
                             int nRemoteSlots,
                             int nLocalUsedSlots = 0,
                             int nRemoteUsedSlots = 0)
    {
        auto localResources = std::make_shared<faabric::HostResources>();
        localResources->set_slots(nLocalSlots);
        localResources->set_usedslots(nLocalUsedSlots);
        sch.addHostToGlobalSet(getDistTestMasterIp(), localResources);

        auto remoteResources = std::make_shared<faabric::HostResources>();
        remoteResources->set_slots(nRemoteSlots);
        remoteResources->set_usedslots(nRemoteUsedSlots);
        sch.addHostToGlobalSet(getDistTestWorkerIp(), remoteResources);
    }

  protected:
    faabric::redis::Redis& redis;
    faabric::scheduler::Scheduler& sch;
    faabric::planner::PlannerClient& plannerCli;
    faabric::util::SystemConfig& conf;
    conf::FaasmConfig& faasmConf;

    int functionCallTimeout = 60000;

    std::string masterIp;
    std::string workerIp;
};

class MpiDistTestsFixture : public DistTestsFixture
{
  public:
    // Given the main MPI message (rank == 0) wait for that message and all the
    // chained messages, and return the result for the main one
    faabric::Message getMpiBatchResult(const faabric::Message& firstMsg,
                                       bool skipChainedCheck = false)
    {
        int appId = firstMsg.appid();
        int firstMsgId = firstMsg.id();
        faabric::Message result =
          plannerCli.getMessageResult(appId, firstMsgId, functionCallTimeout);
        REQUIRE(result.returnvalue() == 0);
        // Wait for all chained messages too
        for (const int chainedId :
             faabric::util::getChainedFunctions(firstMsg)) {
            auto chainedResult =
              plannerCli.getMessageResult(appId, chainedId, functionCallTimeout);
            if (!skipChainedCheck) {
                REQUIRE(chainedResult.returnvalue() == 0);
            }
        }

        return result;
    }

    void checkSchedulingFromExecGraph(
      const faabric::util::ExecGraph& execGraph,
      const std::vector<std::string> expectedHosts)
    {
        std::vector<std::string> hostForRank =
          faabric::util::getMpiRankHostsFromExecGraph(execGraph);
        REQUIRE(expectedHosts == hostForRank);
    }

    void checkSchedulingFromExecGraph(
      const faabric::util::ExecGraph& execGraph,
      const std::vector<std::string> expectedHostsBefore,
      const std::vector<std::string> expectedHostsAfter)
    {
        std::vector<std::string> actualHostsBefore(
          execGraph.rootNode.msg.mpiworldsize());
        std::vector<std::string> actualHostsAfter(
          execGraph.rootNode.msg.mpiworldsize());

        auto actualHostsBeforeAndAfter =
          faabric::util::getMigratedMpiRankHostsFromExecGraph(execGraph);

        REQUIRE(actualHostsBeforeAndAfter.first == expectedHostsBefore);
        REQUIRE(actualHostsBeforeAndAfter.second == expectedHostsAfter);
    }
};

}
