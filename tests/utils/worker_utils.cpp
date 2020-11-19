#include <catch2/catch.hpp>
#include <emulator/emulator.h>
#include <faabric/util/bytes.h>
#include <faabric/util/environment.h>
#include <module_cache/WasmModuleCache.h>

#include <faaslet/FaasletPool.h>

#include "utils.h"

using namespace faaslet;

namespace tests {

Faaslet execFunction(faabric::Message& call, const std::string& expectedOutput)
{
    // Turn off python preloading
    faabric::util::SystemConfig& conf = faabric::util::getSystemConfig();
    std::string originalPreload = conf.pythonPreload;
    conf.pythonPreload = "off";

    // Set up worker to listen for relevant function
    FaasletPool pool(1);
    Faaslet w(1);
    REQUIRE(!w.isBound());

    faabric::scheduler::Scheduler& sch = faabric::scheduler::getScheduler();
    auto bindQueue = sch.getBindQueue();

    // Call the function, checking that everything is set up
    sch.callFunction(call);
    REQUIRE(sch.getFunctionInFlightCount(call) == 1);
    REQUIRE(sch.getFunctionWarmNodeCount(call) == 1);
    REQUIRE(bindQueue->size() == 1);

    // Process the bind message
    w.processNextMessage();
    REQUIRE(w.isBound());
    REQUIRE(sch.getFunctionInFlightCount(call) == 1);
    REQUIRE(sch.getFunctionWarmNodeCount(call) == 1);
    REQUIRE(bindQueue->size() == 0);

    // Now execute the function
    w.processNextMessage();
    REQUIRE(sch.getFunctionInFlightCount(call) == 0);
    REQUIRE(sch.getFunctionWarmNodeCount(call) == 1);
    REQUIRE(bindQueue->size() == 0);

    // Check success
    faabric::Message result = sch.getFunctionResult(call.id(), 1);
    REQUIRE(result.returnvalue() == 0);

    if (!expectedOutput.empty()) {
        REQUIRE(result.outputdata() == expectedOutput);
    }

    conf.pythonPreload = originalPreload;

    return w;
}

std::string execFunctionWithStringResult(faabric::Message& call)
{
    // Turn off python preloading
    faabric::util::SystemConfig& conf = faabric::util::getSystemConfig();
    std::string originalPreload = conf.pythonPreload;
    conf.pythonPreload = "off";

    // Set up worker to listen for relevant function
    FaasletPool pool(1);
    Faaslet w(1);
    REQUIRE(!w.isBound());

    // Call the function
    faabric::scheduler::Scheduler& sch = faabric::scheduler::getScheduler();
    sch.callFunction(call);

    // Process the bind and execute messages
    w.processNextMessage();
    w.processNextMessage();

    const faabric::Message result = sch.getFunctionResult(call.id(), 1);
    if (result.returnvalue() != 0) {
        const std::shared_ptr<spdlog::logger>& logger =
          faabric::util::getLogger();
        logger->error("Function failed: {}", result.outputdata());
        FAIL();
    }
    REQUIRE(result.returnvalue() == 0);

    conf.pythonPreload = originalPreload;

    return result.outputdata();
}

void checkMultipleExecutions(faabric::Message& msg, int nExecs)
{
    module_cache::WasmModuleCache& registry =
      module_cache::getWasmModuleCache();
    wasm::WAVMWasmModule& cachedModule = registry.getCachedModule(msg);

    wasm::WAVMWasmModule module(cachedModule);

    for (int i = 0; i < nExecs; i++) {
        bool success = module.execute(msg);
        REQUIRE(success);

        // Reset
        module = cachedModule;
    }
}

void execFuncWithPool(faabric::Message& call,
                      bool pythonPreload,
                      int repeatCount,
                      bool checkChained,
                      int nThreads,
                      bool clean)
{
    if (clean) {
        cleanSystem();
    }

    setEmulatedMessage(call);

    faabric::scheduler::Scheduler& sch = faabric::scheduler::getScheduler();
    sch.clear();
    sch.addHostToGlobalSet();
    sch.setTestMode(true);

    // Modify system config (network ns requires root)
    faabric::util::SystemConfig& conf = faabric::util::getSystemConfig();
    std::string originalNsMode = conf.netNsMode;
    std::string originalPreload = conf.pythonPreload;
    conf.boundTimeout = 1000;
    conf.unboundTimeout = 1000;
    conf.netNsMode = "off";
    conf.pythonPreload = pythonPreload ? "on" : "off";

    // Set up a real worker pool to execute the function
    conf.maxNodes = nThreads;
    conf.maxNodesPerFunction = nThreads;
    faaslet::FaasletPool pool(nThreads);
    pool.startThreadPool();

    unsigned int mainFuncId;
    for (int i = 0; i < repeatCount; i++) {
        // Reset call ID
        call.set_id(0);
        faabric::util::setMessageId(call);

        mainFuncId = call.id();
        setEmulatedMessage(call);

        // Make the call
        sch.callFunction(call);

        // Await the result of the main function
        // NOTE - this timeout will only get hit when things have failed.
        // It also needs to be long enough to let longer tests complete
        faabric::Message result = sch.getFunctionResult(mainFuncId, 30000);
        REQUIRE(result.returnvalue() == 0);
    }

    // Get all call statuses
    if (checkChained) {
        for (auto messageId : sch.getRecordedMessagesAll()) {
            if (messageId == mainFuncId) {
                // Already checked the main message ID
                continue;
            }

            try {
                const faabric::Message& result =
                  sch.getFunctionResult(messageId, 1);

                if (result.returnvalue() != 0) {
                    FAIL(fmt::format("Message ID {} failed", messageId));
                }
            } catch (faabric::redis::RedisNoResponseException& ex) {
                FAIL(fmt::format("No result for message ID {}", messageId));
            }
        }
    }

    // Shut down the pool
    pool.shutdown();

    conf.netNsMode = originalNsMode;
    conf.pythonPreload = originalPreload;

    cleanSystem();
}

void doWamrPoolExecution(faabric::Message& msg)
{
    faabric::util::SystemConfig& conf = faabric::util::getSystemConfig();
    const std::string originalVm = conf.wasmVm;
    conf.wasmVm = "wamr";

    // Don't clean so that the WAMR configuration persists
    execFuncWithPool(msg, false, 1, false, 5, false);

    conf.wasmVm = originalVm;
}

void executeWithWamrPool(const std::string& user, const std::string& func)
{
    faabric::Message call = faabric::util::messageFactory(user, func);
    doWamrPoolExecution(call);
}

void executeWithSgx(const std::string& user, const std::string& func)
{
    faabric::Message call = faabric::util::messageFactory(user, func);
    call.set_issgx(true);
    doWamrPoolExecution(call);
}

void checkCallingFunctionGivesBoolOutput(const std::string& user,
                                         const std::string& funcName,
                                         bool expected)
{
    faabric::Message call = faabric::util::messageFactory("demo", funcName);
    setEmulatedMessage(call);

    FaasletPool pool(1);
    Faaslet w(1);

    faabric::scheduler::Scheduler& sch = faabric::scheduler::getScheduler();
    sch.callFunction(call);

    // Bind and execute
    w.processNextMessage();
    w.processNextMessage();

    // Check output is true
    faabric::Message result = sch.getFunctionResult(call.id(), 1);
    REQUIRE(result.returnvalue() == 0);
    std::vector<uint8_t> outputBytes =
      faabric::util::stringToBytes(result.outputdata());

    std::vector<uint8_t> expectedOutput;

    if (expected) {
        expectedOutput = { 1 };
    } else {
        expectedOutput = { 0 };
    }

    REQUIRE(outputBytes == expectedOutput);
}
}
