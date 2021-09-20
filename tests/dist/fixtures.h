#pragma once

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
    faabric::scheduler::Scheduler& sch;
    faabric::util::SystemConfig& conf;
    conf::FaasmConfig& faasmConf;

    std::string masterIp;
    std::string workerIp;

  public:
    DistTestsFixture()
      : sch(faabric::scheduler::getScheduler())
      , conf(faabric::util::getSystemConfig())
      , faasmConf(conf::getFaasmConfig())
    {
        // Clean up the scheduler and make sure this host is available
        sch.shutdown();
        sch.addHostToGlobalSet();
        sch.addHostToGlobalSet(getDistTestWorkerIp());

        // Set up executor
        std::shared_ptr<faaslet::FaasletFactory> fac =
          std::make_shared<faaslet::FaasletFactory>();
        faabric::scheduler::setExecutorFactory(fac);
    }

    std::string getDistTestMasterIp()
    {
        if (masterIp.empty()) {
            masterIp = faabric::util::getIPFromHostname("faasm-cli");
        }
        return masterIp;
    }

    std::string getDistTestWorkerIp()
    {
        if (workerIp.empty()) {
            workerIp = faabric::util::getIPFromHostname("dist-test-server");
        }
        return workerIp;
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
