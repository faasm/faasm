#pragma once

#include <catch2/catch.hpp>

#include "faabric_utils.h"

#include <codegen/MachineCodeGenerator.h>
#include <conf/FaasmConfig.h>
#include <faabric/planner/PlannerClient.h>
#include <faabric/scheduler/Scheduler.h>
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

    std::string getDistTestMasterIp() const { return conf.endpointHost; }

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
    void checkMpiBatchResults(std::shared_ptr<faabric::BatchExecuteRequest> req,
                              const std::vector<std::string>& expectedHosts)
    {
        int expectedWorldSize = req->messages(0).mpiworldsize();

        // First, poll untill all messages are ready
        int pollSleepSecs = 2;
        auto batchResults = plannerCli.getBatchResults(req);
        while (batchResults->messageresults_size() != expectedWorldSize) {
            SLEEP_MS(pollSleepSecs * 1000);
            batchResults = plannerCli.getBatchResults(req);
        }

        for (const auto& msg : batchResults->messageresults()) {
            REQUIRE(msg.returnvalue() == 0);
            REQUIRE(expectedHosts.at(msg.mpirank()) == msg.executedhost());
        }
    }

    // Wait until `mpiworldsize` messages are in-flight for a given request
    // and return the intial allocation. This method can be used to
    // make sure that the first module has been instantaited, it has
    // chained the remaining ranks, and the planner has scheduled them
    std::vector<std::string> waitForMpiMessagesInFlight(
      std::shared_ptr<BatchExecuteRequest> req)
    {
        int maxRetries = 20;
        int numRetries = 0;
        int expectedWorldSize = req->messages(0).mpiworldsize();
        auto decision = plannerCli.getSchedulingDecision(req);
        while (decision.messageIds.size() != expectedWorldSize) {
            if (numRetries >= maxRetries) {
                SPDLOG_ERROR(
                  "Timed-out waiting for MPI messages to be scheduled ({}/{})",
                  decision.messageIds.size(),
                  expectedWorldSize);
                throw std::runtime_error("Timed-out waiting for MPI messges");
            }

            SPDLOG_DEBUG("Waiting for MPI messages to be scheduled ({}/{})",
                         decision.messageIds.size(),
                         expectedWorldSize);
            SLEEP_MS(500);

            numRetries += 1;
            decision = plannerCli.getSchedulingDecision(req);
        }

        return decision.hosts;
    }
};

}
