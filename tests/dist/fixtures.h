#pragma once

#include <catch2/catch.hpp>

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

        // Clean up the scheduler and make sure this host is available
        sch.shutdown();
        sch.addHostToGlobalSet();
        sch.addHostToGlobalSet(getDistTestWorkerIp());

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

        faabric::scheduler::getMigratedMpiRankHostsFromExecGraph(execGraph,
                                                                 actualHostsBefore,
                                                                 actualHostsAfter);

        REQUIRE(actualHostsBefore == expectedHostsBefore);
        REQUIRE(actualHostsAfter == expectedHostsAfter);
    }
};

}
