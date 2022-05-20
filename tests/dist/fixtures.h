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

    // TODO - this method is duplicated from faabric's dist tests
    void checkSchedulingFromExecGraph(
      const faabric::scheduler::ExecGraph& execGraph,
      const std::vector<std::string> expectedHostsBefore,
      const std::vector<std::string> expectedHostsAfter)
    {
        std::vector<std::string> actualHostsBefore(
          execGraph.rootNode.msg.mpiworldsize());
        std::vector<std::string> actualHostsAfter(
          execGraph.rootNode.msg.mpiworldsize());

        std::queue<faabric::scheduler::ExecGraphNode> nodeList;
        nodeList.push(execGraph.rootNode);
        while (!nodeList.empty()) {
            // Process the node at the front
            auto node = nodeList.front();
            int returnValue = node.msg.returnvalue();
            int rank = node.msg.mpirank();
            std::string executedHost = node.msg.executedhost();
            if (returnValue == 0) {
                // We don't know if this particular rank has been migrated or
                // not. Thus we only write in the before vector if no-one has
                // written to that rank before
                if (actualHostsBefore.at(rank).empty()) {
                    actualHostsBefore.at(rank) = executedHost;
                }
                actualHostsAfter.at(rank) = executedHost;
            } else if (returnValue == MIGRATED_FUNCTION_RETURN_VALUE) {
                // When we process a message that has been migrated we always
                // overwrite the contents of the before vector
                actualHostsBefore.at(rank) = executedHost;
            } else {
                SPDLOG_ERROR("Unexpected return value {} for message id {}",
                             returnValue,
                             node.msg.id());
                throw std::runtime_error("Unexpected return value");
            }
            nodeList.pop();

            // Add children to the queue
            for (auto c : node.children) {
                nodeList.push(c);
            }
        }

        REQUIRE(actualHostsBefore == expectedHostsBefore);
        REQUIRE(actualHostsAfter == expectedHostsAfter);
    }
};

}
