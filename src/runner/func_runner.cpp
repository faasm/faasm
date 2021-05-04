#include <faaslet/Faaslet.h>
#include <wasm/WasmModule.h>

#include <faabric/redis/Redis.h>
#include <faabric/runner/FaabricMain.h>
#include <faabric/scheduler/ExecutorFactory.h>
#include <faabric/util/config.h>
#include <faabric/util/timing.h>

int main(int argc, char* argv[])
{
    const auto& logger = faabric::util::getLogger();

    if (argc < 3) {
        logger->error("Must provide user and function name");
        return 1;
    }

    faabric::util::SystemConfig& conf = faabric::util::getSystemConfig();

    // Set short timeouts to die quickly
    conf.boundTimeout = 60000;
    conf.globalMessageTimeout = 60000;
    conf.chainedCallTimeout = 60000;

    // Make sure we have enough space for chained calls
    int nThreads = 10;
    faabric::HostResources res;
    res.set_slots(nThreads);
    faabric::scheduler::Scheduler& sch = faabric::scheduler::getScheduler();
    sch.setThisHostResources(res);

    conf.defaultMpiWorldSize = 5;

    // Clear out redis
    faabric::redis::Redis& redis = faabric::redis::Redis::getQueue();
    redis.flushAll();

    // Set up the call
    std::string user = argv[1];
    std::string function = argv[2];
    faabric::Message call = faabric::util::messageFactory(user, function);

    if (user == "python") {
        faabric::util::convertMessageToPython(call);
        logger->info("Running Python function {}/{}",
                     call.pythonuser(),
                     call.pythonfunction());
    } else {
        logger->info("Running function {}/{}", user, function);
    }

    if (argc > 3) {
        std::string inputData = argv[3];
        call.set_inputdata(inputData);

        logger->info("Adding input data: {}", inputData);
    }

    // Set up the system
    auto fac = std::make_shared<faaslet::FaasletFactory>();
    faabric::runner::FaabricMain m(fac);
    m.startRunner();

    // Submit the invocation
    PROF_START(roundTrip)
    sch.callFunction(call);

    // Await the result
    const faabric::Message& result =
      sch.getFunctionResult(call.id(), conf.globalMessageTimeout);
    if (result.returnvalue() != 0) {
        logger->error("Execution failed: {}", result.outputdata());
        throw std::runtime_error("Executing function failed");
    }

    PROF_END(roundTrip)

    m.shutdown();

    return 0;
}
