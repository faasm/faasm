#include "faabric/transport/context.h"
#include "faaslet/Faaslet.h"
#include "storage/S3Wrapper.h"
#include <filesystem>

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <faabric/transport/context.h>
#include <faabric/util/crash.h>
#include <faabric/util/logging.h>

#include <faabric/scheduler/Scheduler.h>
#include <faabric/util/config.h>
#include <faabric/util/environment.h>
#include <faabric/util/files.h>
#include <faabric/util/func.h>
#include <faabric/util/timing.h>

#include <conf/FaasmConfig.h>
#include <wamr/WAMRWasmModule.h>
#include <wasm/WasmModule.h>
#include <wavm/WAVMWasmModule.h>

void execCovid(int nThreads, int nLoops, const std::string& country)
{
    SPDLOG_INFO(
      "Running covid sim with {} threads, {} loops", nThreads, nLoops);

    std::string cmdlineArgs = "/c:" + std::to_string(nThreads) + " ";

    if (country == "Guam") {
        cmdlineArgs += "/A:faasm://covid/admin_units/Guam_admin.txt "
                       "/PP:faasm://covid/param_files/preUK_R0=2.0.txt "
                       "/P:faasm://covid/param_files/p_NoInt.txt "
                       "/O:/tmp/Guam_NoInt_R0=3.0 "
                       "/D:/faasm://covid/populations/wpop_us_terr.txt "
                       "/M:/tmp/Guam_pop_density.bin "
                       "/S:/tmp/Network_Guam_T1_R3.0.bin "
                       "/R:1.5 98798150 729101 17389101 4797132";
    } else if (country == "Virgin_Islands_US") {
        cmdlineArgs +=
          "/A:faasm://covid/admin_units/Virgin_Islands_US_admin.txt "
          "/PP:faasm://covid/param_files/preUK_R0=2.0.txt "
          "/P:faasm://covid/param_files/p_NoInt.txt "
          "/O:/tmp/Virgin_Islands_US_NoInt_R0=3.0 "
          "/D:/faasm://covid/populations/wpop_us_terr.txt "
          "/M:/tmp/Virgin_Islands_US_pop_density.bin "
          "/S:/tmp/Network_Virgin_Islands_US_T1_R3.0.bin "
          "/R:1.5 98798150 729101 17389101 4797132";
    } else if (country == "Malta") {
        cmdlineArgs += "/A:faasm://covid/admin_units/Malta_admin.txt "
                       "/PP:faasm://covid/param_files/preUK_R0=2.0.txt "
                       "/P:faasm://covid/param_files/p_NoInt.txt "
                       "/O:/tmp/Virgin_Islands_US_NoInt_R0=3.0 "
                       "/D:/faasm://covid/populations/wpop_eur.txt "
                       "/M:/tmp/Malta_pop_density.bin "
                       "/S:/tmp/Network_Malta_T1_R3.0.bin "
                       "/R:1.5 98798150 729101 17389101 4797132";
    } else {
        throw std::runtime_error("Unrecognised country");
    }

    faabric::Message msg = faabric::util::messageFactory("cov", "sim");
    msg.set_cmdline(cmdlineArgs);

    // Set short timeouts to die quickly
    faabric::util::SystemConfig& conf = faabric::util::getSystemConfig();
    conf.boundTimeout = 480000;
    conf.globalMessageTimeout = 480000;

    conf.print();

    // Clear out redis
    faabric::redis::Redis& redis = faabric::redis::Redis::getQueue();
    redis.flushAll();

    // Run repeated executions
    for (int i = 0; i < nLoops; i++) {
        wasm::WAVMWasmModule module;
        module.bindToFunction(msg);

        int returnValue = module.executeFunction(msg);
        if (returnValue != 0) {
            module.printDebugInfo();
            SPDLOG_ERROR("Execution failed");
            break;
        }
    }
}

int main(int argc, char* argv[])
{
    faabric::util::setUpCrashHandler();

    faabric::util::initLogging();
    storage::initFaasmS3();
    faabric::transport::initGlobalMessageContext();

    std::shared_ptr<faaslet::FaasletFactory> fac =
      std::make_shared<faaslet::FaasletFactory>();
    faabric::scheduler::setExecutorFactory(fac);

    std::string country = "Guam";
    if (argc >= 4) {
        country = argv[3];
    }

    int nLoops = 1;
    if (argc >= 3) {
        nLoops = std::stoi(argv[2]);
    }

    int nThreads = faabric::util::getUsableCores();
    if (argc >= 2) {
        nThreads = std::stoi(argv[1]);
    }

    execCovid(nThreads, nLoops, country);

    faabric::transport::closeGlobalMessageContext();

    storage::shutdownFaasmS3();

    return 0;
}
