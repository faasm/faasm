#include "utils.h"
#include <catch2/catch.hpp>

#include <faabric/proto/faabric.pb.h>
#include <faabric/scheduler/FunctionCallClient.h>
#include <faabric/scheduler/SnapshotClient.h>
#include <faabric/snapshot/SnapshotRegistry.h>
#include <faabric/util/bytes.h>
#include <faabric/util/environment.h>
#include <faabric/util/testing.h>
#include <faaslet/FaasletPool.h>
#include <module_cache/WasmModuleCache.h>
#include <wavm/WAVMWasmModule.h>

using namespace faaslet;

namespace tests {

void execFunction(faabric::Message& call, const std::string& expectedOutput)
{
    // Turn off python preloading
    faabric::util::SystemConfig& conf = faabric::util::getSystemConfig();
    std::string originalPreload = conf.pythonPreload;
    conf.pythonPreload = "off";

    wasm::WAVMWasmModule module;
    module.bindToFunction(call);
    bool success = module.execute(call);

    if (!expectedOutput.empty()) {
        std::string actualOutput = call.outputdata();
        REQUIRE(actualOutput == expectedOutput);
    }

    REQUIRE(success);
    REQUIRE(call.returnvalue() == 0);

    conf.pythonPreload = originalPreload;
}

std::string execFunctionWithStringResult(faabric::Message& call)
{
    // Turn off python preloading
    faabric::util::SystemConfig& conf = faabric::util::getSystemConfig();
    std::string originalPreload = conf.pythonPreload;
    conf.pythonPreload = "off";

    // Set up worker to listen for relevant function
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

void execFunctionWithRemoteBatch(faabric::Message& call,
                                 int nThreads,
                                 bool clean)
{
    if (clean) {
        cleanSystem();
    }

    faabric::util::setMockMode(true);

    faabric::scheduler::Scheduler& sch = faabric::scheduler::getScheduler();

    // Add other host to available hosts
    std::string otherHost = "other";
    sch.addHostToGlobalSet(otherHost);

    // Set up other host to have some resources
    faabric::HostResources resOther;
    resOther.set_cores(10);
    faabric::scheduler::queueResourceResponse(otherHost, resOther);

    // Make sure we have no cores so we get distribution
    int nCores = 0;
    faabric::HostResources res;
    res.set_cores(nCores);
    sch.setThisHostResources(res);

    // Background thread to execute function (won't finish until threads have
    // been executed)
    std::thread t([&call] {
        wasm::WAVMWasmModule module;
        module.bindToFunction(call);
        bool success = module.execute(call);

        REQUIRE(success);
    });

    // Give it time to have made the request
    usleep(1000 * 500);

    auto reqs = faabric::scheduler::getBatchRequests();
    REQUIRE(reqs.size() == 1);
    std::string actualHost = reqs.at(0).first;
    faabric::BatchExecuteRequest req = reqs.at(0).second;
    REQUIRE(actualHost == otherHost);

    // Check the snapshot has been pushed to the other host
    auto snapPushes = faabric::scheduler::getSnapshotPushes();
    REQUIRE(snapPushes.size() == 1);
    REQUIRE(snapPushes.at(0).first == otherHost);

    // Rewrite the snapshot to be restorable. This is a bit of a hack, usually
    // this would have been done via the RPC call between the hosts.
    faabric::Message firstMsg = req.messages().at(0);
    std::string snapKey = firstMsg.snapshotkey();
    faabric::snapshot::SnapshotRegistry& reg =
      faabric::snapshot::getSnapshotRegistry();
    faabric::util::SnapshotData& snapData = reg.getSnapshot(snapKey);
    reg.takeSnapshot(snapKey, snapData);

    // Now execute request on this host (forced)
    // Note - don't clean as we will have already done at the top of this func
    execBatchWithPool(req, nThreads, false, false);

    if (t.joinable()) {
        t.join();
    }
}

void execBatchWithPool(faabric::BatchExecuteRequest& req,
                       int nThreads,
                       bool checkChained,
                       bool clean)
{
    if (clean) {
        cleanSystem();
    }

    faabric::util::SystemConfig& conf = faabric::util::getSystemConfig();
    conf.boundTimeout = 1000;
    conf.unboundTimeout = 1000;
    conf.chainedCallTimeout = 10000;

    faabric::scheduler::Scheduler& sch = faabric::scheduler::getScheduler();

    // Start a Faaslet pool to execute things
    faaslet::FaasletPool pool(nThreads);
    pool.startThreadPool();

    // Execute forcing local
    sch.callFunctions(req, true);

    usleep(1000 * 500);

    // Wait for all functions to complete if necessary
    if (checkChained) {
        for (auto m : req.messages()) {
            faabric::Message result = sch.getFunctionResult(m.id(), 20000);
            REQUIRE(result.returnvalue() == 0);
        }
    }

    pool.shutdown();
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

    faabric::scheduler::Scheduler& sch = faabric::scheduler::getScheduler();
    sch.shutdown();
    sch.addHostToGlobalSet();

    // Modify system config (network ns requires root)
    faabric::util::SystemConfig& conf = faabric::util::getSystemConfig();
    std::string originalNsMode = conf.netNsMode;
    std::string originalPreload = conf.pythonPreload;
    conf.boundTimeout = 1000;
    conf.unboundTimeout = 1000;
    conf.netNsMode = "off";
    conf.pythonPreload = pythonPreload ? "on" : "off";

    // Set up a real worker pool to execute the function
    faaslet::FaasletPool pool(nThreads);
    pool.startThreadPool();

    unsigned int mainFuncId;
    for (int i = 0; i < repeatCount; i++) {
        // Reset call ID
        call.set_id(0);
        faabric::util::setMessageId(call);

        mainFuncId = call.id();

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

void executeWithSGX(const std::string& user, const std::string& func)
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
