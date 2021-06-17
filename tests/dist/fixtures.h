#pragma once

#include <conf/FaasmConfig.h>
#include <faabric/scheduler/Scheduler.h>
#include <faaslet/Faaslet.h>

#define MASTER_IP "172.60.0.4"
#define WORKER_IP "172.60.0.5"

namespace tests {
class DistTestsFixture
{
  protected:
    faabric::scheduler::Scheduler& sch;
    faabric::util::SystemConfig& conf;
    conf::FaasmConfig& faasmConf;
    std::set<std::string> otherHosts;

  public:
    DistTestsFixture()
      : sch(faabric::scheduler::getScheduler())
      , conf(faabric::util::getSystemConfig())
      , faasmConf(conf::getFaasmConfig())
    {
        // Clean up the scheduler and make sure this host is available
        sch.shutdown();
        sch.addHostToGlobalSet();

        // Get other hosts
        std::string thisHost = conf.endpointHost;
        otherHosts = sch.getAvailableHosts();
        otherHosts.erase(thisHost);

        // Set up executor
        std::shared_ptr<faaslet::FaasletFactory> fac =
          std::make_shared<faaslet::FaasletFactory>();
        faabric::scheduler::setExecutorFactory(fac);
    }

    ~DistTestsFixture()
    {
        sch.broadcastFlush();
        conf.reset();
        faasmConf.reset();
    }
};
}
