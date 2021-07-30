#pragma once

#include <codegen/MachineCodeGenerator.h>
#include <conf/FaasmConfig.h>
#include <faaslet/Faaslet.h>
#include <storage/FileLoader.h>

#include <faabric/scheduler/Scheduler.h>
#include <faabric/util/func.h>

#define MASTER_IP "172.60.0.6"
#define WORKER_IP "172.60.0.7"

namespace tests {
class DistTestsFixture
{
  protected:
    faabric::scheduler::Scheduler& sch;
    faabric::util::SystemConfig& conf;
    conf::FaasmConfig& faasmConf;

  public:
    DistTestsFixture()
      : sch(faabric::scheduler::getScheduler())
      , conf(faabric::util::getSystemConfig())
      , faasmConf(conf::getFaasmConfig())
    {
        // Clean up the scheduler and make sure this host is available
        sch.shutdown();
        sch.addHostToGlobalSet();
        sch.addHostToGlobalSet(WORKER_IP);

        // Set up executor
        std::shared_ptr<faaslet::FaasletFactory> fac =
          std::make_shared<faaslet::FaasletFactory>();
        faabric::scheduler::setExecutorFactory(fac);
    }

    void uploadExistingFunction(const std::string& user,
                                const std::string& func)
    {
        // Although we may have the function data locally, we need to upload it
        // again to make sure it's written to S3
        faabric::Message msg = faabric::util::messageFactory(user, func);

        storage::FileLoader& loader = storage::getFileLoader();
        codegen::MachineCodeGenerator& gen = codegen::getMachineCodeGenerator();

        std::vector<uint8_t> bytes = loader.loadFunctionWasm(msg);
        msg.set_inputdata(bytes.data(), bytes.size());

        loader.uploadFunction(msg);
        gen.codegenForFunction(msg);
    }

    ~DistTestsFixture()
    {
        sch.broadcastFlush();
        conf.reset();
        faasmConf.reset();
    }
};
}
