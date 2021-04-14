#include <filesystem>

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

#include <conf/FaasmConfig.h>
#include <faabric/scheduler/Scheduler.h>
#include <faabric/util/config.h>
#include <faabric/util/environment.h>
#include <faabric/util/files.h>
#include <faabric/util/func.h>
#include <faaslet/FaasletPool.h>
#include <module_cache/WasmModuleCache.h>
#include <wamr/WAMRWasmModule.h>
#include <wasm/WasmModule.h>

void execCovid(int nThreads, int nLoops)
{
    auto logger = faabric::util::getLogger();
    logger->info(
      "Running covid sim with {} threads, {} loops", nThreads, nLoops);

    std::string cmdlineArgs = "/c:" + std::to_string(nThreads);

    cmdlineArgs +=
      " /A:faasm://covid/admin_units/Guam_admin.txt "
      "/PP:faasm://covid/param_files/preUK_R0=2.0.txt "
      "/P:faasm://covid/param_files/p_NoInt.txt /O:/tmp/Guam_NoInt_R0=3.0 "
      "/D:/faasm://covid/populations/wpop_us_terr.txt "
      "/M:/tmp/Guam_pop_density.bin /S:/tmp/Network_Guam_T1_R3.0.bin "
      "/R:1.5 98798150 729101 17389101 4797132";

    faabric::Message msg = faabric::util::messageFactory("cov", "sim");
    msg.set_cmdline(cmdlineArgs);

    // Set short timeouts to die quickly
    faabric::util::SystemConfig& conf = faabric::util::getSystemConfig();
    conf.boundTimeout = 480000;
    conf.unboundTimeout = 480000;
    conf.globalMessageTimeout = 480000;
    conf.chainedCallTimeout = 480000;

    conf.print();

    // Clear out redis
    faabric::redis::Redis& redis = faabric::redis::Redis::getQueue();
    redis.flushAll();

    // Force the Faasm thread pool size
    logger->debug("Setting Faasm thread pool size to {}", nThreads);
    conf::getFaasmConfig().moduleThreadPoolSize = nThreads;

    std::string tmpDirA = "/usr/local/faasm/runtime_root/tmp";
    std::string tmpDirB =
      "/usr/local/code/faasm/dev/faasm-local/runtime_root/tmp";

    module_cache::WasmModuleCache& registry =
      module_cache::getWasmModuleCache();
    wasm::WAVMWasmModule& cachedModule = registry.getCachedModule(msg);

    // Create new module from cache
    wasm::WAVMWasmModule module(cachedModule);

    // Run repeated executions
    for (int i = 0; i < nLoops; i++) {
        std::string tmpDir;
        if (std::filesystem::exists(tmpDirA)) {
            tmpDir = tmpDirA;
        } else if (std::filesystem::exists(tmpDirB)) {
            tmpDir = tmpDirB;
        } else {
            throw std::runtime_error("tmp dir not found");
        }

        logger->debug("Clearing tmp dir {}", tmpDir);
        std::filesystem::remove_all(tmpDir);
        std::filesystem::create_directory(tmpDir);

        bool success = module.execute(msg);
        if (!success) {
            module.printDebugInfo();
            logger->error("Execution failed");
            break;
        }

        // Reset using cached module
        module = cachedModule;
    }
}

int main(int argc, char* argv[])
{
    auto logger = faabric::util::getLogger();

    int nLoops = 1;
    if (argc >= 3) {
        nLoops = std::stoi(argv[2]);
    }

    int nThreads = faabric::util::getUsableCores();
    if (argc >= 2) {
        nThreads = std::stoi(argv[1]);
    }

    execCovid(nThreads, nLoops);

    return 0;
}
