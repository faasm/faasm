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
  protected:
    faabric::redis::Redis& redis;
    faabric::scheduler::Scheduler& sch;
    faabric::util::SystemConfig& conf;
    conf::FaasmConfig& faasmConf;

    int functionCallTimeout = 60000;

    std::string masterIp;
    std::string workerIp;

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
        resetPlanner();

        // Clean up the scheduler
        sch.shutdown();

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
        sch.broadcastFlush();
        conf.reset();
        faasmConf.reset();
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
};

class MpiDistTestsFixture : public DistTestsFixture
{
  public:
    void checkSchedulingFromExecGraph(
      const faabric::scheduler::ExecGraph& execGraph,
      const std::vector<std::string> expectedHosts)
    {
        std::vector<std::string> hostForRank =
          faabric::scheduler::getMpiRankHostsFromExecGraph(execGraph);
        REQUIRE(expectedHosts == hostForRank);
    }

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
