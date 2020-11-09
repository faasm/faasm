#include <faabric/util/logging.h>

#include <faaslet/FaasletPool.h>

#include <faabric/endpoint/FaabricEndpoint.h>
#include <faabric/executor/FaabricMain.h>

using namespace faabric::executor;
using namespace faaslet;

int main()
{
    faabric::util::initLogging();
    const std::shared_ptr<spdlog::logger>& logger = faabric::util::getLogger();

    // Set up the message
    auto msg = faabric::util::messageFactory("lammps", "test");
    msg.set_cmdline("-in faasm://lammps-data/in.controller");
    logger->info("Running function lammps/test");

    // Set up the scheduler
    faabric::scheduler::Scheduler& sch = faabric::scheduler::getScheduler();
    sch.clear();
    sch.addHostToGlobalSet();

    // Set up the configuration
    faabric::util::SystemConfig &conf = faabric::util::getSystemConfig();
    int nThreads = 5;
    //conf.defaultMpiWorldSize = 5;
    conf.maxNodes = nThreads;
    conf.maxNodesPerFunction = nThreads;

    // Start the worker pool
    logger->info("Starting faaslet pool in the background");
    FaasletPool pool(5);
    pool.startThreadPool();

    // Run LAMMPS
    sch.callFunction(msg);

    // Await the result
    const faabric::Message& result =
      sch.getFunctionResult(msg.id(), conf.globalMessageTimeout);
    if (result.returnvalue() != 0) {
        logger->error("Execution failed: {}", result.outputdata());
        throw std::runtime_error("Executing function failed");
    }

    // Shutdown thread pool
    pool.shutdown();

    // FaabricMain w(p);
    // w.startBackground();

    // Start endpoint (will also have multiple threads)
    //    logger->info("Starting endpoint");
    //    faabric::endpoint::FaabricEndpoint endpoint;
    //    endpoint.start();
    //
    //    logger->info("Shutting down endpoint");
    //    w.shutdown();

    return EXIT_SUCCESS;
}
