#include <wasm/WasmModule.h>

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <faabric/scheduler/Scheduler.h>
#include <faabric/util/config.h>
#include <faabric/util/func.h>
#include <faaslet/FaasletPool.h>

#include <module_cache/WasmModuleCache.h>
#include <wamr/WAMRWasmModule.h>

#include <faabric/util/files.h>

int main(int argc, char* argv[])
{
    auto logger = faabric::util::getLogger();

    faabric::Message msg = faabric::util::messageFactory("cov", "sim");

    int nThreads = 1;

    std::string cmdlineArgs =
      "/c:1 "
      "/A:faasm://covid/admin_units/Guam_admin.txt "
      "/PP:faasm://covid/param_files/preUK_R0=2.0.txt "
      "/P:faasm://covid/param_files/p_NoInt.txt /O:/tmp/Guam_NoInt_R0=3.0 "
      "/D:/faasm://covid/populations/wpop_us_terr.txt "
      "/M:/tmp/Guam_pop_density.bin /S:/tmp/Network_Guam_T1_R3.0.bin "
      "/R:1.5 98798150 729101 17389101 4797132";

    msg.set_cmdline(cmdlineArgs);

    // Set short timeouts to die quickly
    auto conf = faabric::util::getSystemConfig();
    conf.boundTimeout = 240000;
    conf.unboundTimeout = 240000;
    conf.globalMessageTimeout = 240000;
    conf.chainedCallTimeout = 240000;

    conf.print();

    faabric::scheduler::Scheduler& sch = faabric::scheduler::getScheduler();

    // Clear out redis
    faabric::redis::Redis& redis = faabric::redis::Redis::getQueue();
    redis.flushAll();

    // Start pool
    faaslet::FaasletPool pool(nThreads);
    pool.startThreadPool();

    // Execute
    sch.callFunction(msg);

    // Await the result
    const faabric::Message& result =
      sch.getFunctionResult(msg.id(), conf.globalMessageTimeout);
    if (result.returnvalue() != 0) {
        logger->error("Execution failed: {}", result.outputdata());
        throw std::runtime_error("Executing function failed");
    }

    pool.shutdown();
    return 0;
}
