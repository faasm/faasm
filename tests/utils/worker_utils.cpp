#include "utils.h"
#include <catch2/catch.hpp>

#include <faabric/proto/faabric.pb.h>
#include <faabric/runner/FaabricMain.h>
#include <faabric/scheduler/FunctionCallClient.h>
#include <faabric/snapshot/SnapshotClient.h>
#include <faabric/snapshot/SnapshotRegistry.h>
#include <faabric/util/bytes.h>
#include <faabric/util/config.h>
#include <faabric/util/environment.h>
#include <faabric/util/logging.h>
#include <faabric/util/testing.h>
#include <faaslet/Faaslet.h>

#include <conf/FaasmConfig.h>
#ifndef FAASM_SGX_DISABLED_MODE
#include <enclave/outside/EnclaveInterface.h>
#endif
#include <wamr/WAMRWasmModule.h>
#include <wavm/WAVMWasmModule.h>

using namespace faaslet;

namespace tests {

void execFunction(std::shared_ptr<faabric::BatchExecuteRequest> req,
                  const std::string& expectedOutput)
{
    return execFunction(req->mutable_messages()->at(0), expectedOutput);
}

void execFunction(faabric::Message& call, const std::string& expectedOutput)
{
    // Turn off python preloading
    conf::FaasmConfig& conf = conf::getFaasmConfig();
    std::string originalPreload = conf.pythonPreload;
    conf.pythonPreload = "off";

    wasm::WAVMWasmModule module;
    module.bindToFunction(call);
    int returnValue = module.executeFunction(call);

    if (!expectedOutput.empty()) {
        std::string actualOutput = call.outputdata();
        REQUIRE(actualOutput == expectedOutput);
    }

    REQUIRE(returnValue == 0);
    REQUIRE(call.returnvalue() == 0);

    conf.pythonPreload = originalPreload;
}

void execWamrFunction(faabric::Message& call, const std::string& expectedOutput)
{
    // Turn off python preloading
    conf::FaasmConfig& conf = conf::getFaasmConfig();
    std::string originalPreload = conf.pythonPreload;
    conf.pythonPreload = "off";
    conf.wasmVm = "wamr";

    wasm::WAMRWasmModule module;
    module.bindToFunction(call);
    int returnValue = module.executeFunction(call);

    if (!expectedOutput.empty()) {
        std::string actualOutput = call.outputdata();
        REQUIRE(actualOutput == expectedOutput);
    }
    module.reset(call, "");

    REQUIRE(returnValue == 0);
    REQUIRE(call.returnvalue() == 0);

    conf.pythonPreload = originalPreload;
}

std::string execFunctionWithStringResult(faabric::Message& call)
{
    // Turn off python preloading
    conf::FaasmConfig& conf = conf::getFaasmConfig();
    std::string originalPreload = conf.pythonPreload;
    conf.pythonPreload = "off";

    auto fac = std::make_shared<faaslet::FaasletFactory>();
    faabric::runner::FaabricMain m(fac);
    m.startRunner();

    // Convert message to request
    // TODO: do less hacky
    auto req = faabric::util::batchExecFactory(call.user(), call.function(), 1);
    req->set_appid(call.appid());
    *req->mutable_messages(0) = call;

    // Call the function
    faabric::scheduler::Scheduler& sch = faabric::scheduler::getScheduler();
    sch.callFunctions(req);

    // This timeout needs to be long enough for slow functions to execute
    const faabric::Message result = sch.getFunctionResult(call, 20000);
    if (result.returnvalue() != 0) {
        SPDLOG_ERROR("Function failed: {}", result.outputdata());
        FAIL();
    }
    REQUIRE(result.returnvalue() == 0);

    conf.pythonPreload = originalPreload;

    m.shutdown();

    return result.outputdata();
}

void checkMultipleExecutions(faabric::Message& msg, int nExecs)
{
    wasm::WAVMWasmModule module;
    module.bindToFunction(msg);

    for (int i = 0; i < nExecs; i++) {
        int returnValue = module.executeFunction(msg);
        REQUIRE(returnValue == 0);

        // Reset
        module.reset(msg, "");
    }
}

void execBatchWithPool(std::shared_ptr<faabric::BatchExecuteRequest> req,
                       int nSlots)
{
    faabric::util::SystemConfig& conf = faabric::util::getSystemConfig();
    conf::FaasmConfig& faasmConf = conf::getFaasmConfig();
    conf.boundTimeout = 1000;
    faasmConf.chainedCallTimeout = 10000;

    faabric::scheduler::Scheduler& sch = faabric::scheduler::getScheduler();

    // Start up system
    auto fac = std::make_shared<faaslet::FaasletFactory>();
    faabric::runner::FaabricMain m(fac);
    m.startRunner();

    // Execute forcing local
    req->mutable_messages()->at(0).set_topologyhint("FORCE_LOCAL");
    sch.callFunctions(req);

    usleep(1000 * 500);

    // Wait for all functions to complete
    for (const auto& m : req->messages()) {
        if (req->type() == faabric::BatchExecuteRequest::THREADS) {
            int returnValue = sch.awaitThreadResult(m.id());
            REQUIRE(returnValue == 0);
        } else {
            faabric::Message result = sch.getFunctionResult(m, 20000);
            REQUIRE(result.returnvalue() == 0);
        }
    }

    m.shutdown();
}

std::shared_ptr<faabric::BatchExecuteRequest> execFuncWithPool(
  std::shared_ptr<faabric::BatchExecuteRequest> req,
  bool clean,
  int timeout)
{
    faabric::scheduler::Scheduler& sch = faabric::scheduler::getScheduler();
    sch.shutdown();
    sch.addHostToGlobalSet();

    // Modify system config (network ns requires root)
    faabric::util::SystemConfig& conf = faabric::util::getSystemConfig();
    conf::FaasmConfig& faasmConf = conf::getFaasmConfig();
    std::string originalNsMode = faasmConf.netNsMode;
    conf.boundTimeout = timeout;
    faasmConf.netNsMode = "off";

    // Set up the system
    auto fac = std::make_shared<faaslet::FaasletFactory>();
    faabric::runner::FaabricMain m(fac);
    m.startRunner();

    // Make the call
    sch.callFunctions(req);

    // Give the planner time to schedule any calls
    SLEEP_MS(200);

    auto reqResp = sch.getBatchResult(req, timeout);
    for (const auto& msg : reqResp->messages()) {
        REQUIRE(msg.returnvalue() == 0);
    }

    return reqResp;
}

faabric::Message execFuncWithPool(faabric::Message& call,
                                  bool clean,
                                  int timeout)
{
    // Convert message to request
    // TODO: do less hacky
    auto req = faabric::util::batchExecFactory(call.user(), call.function(), 1);
    req->set_appid(call.appid());
    *req->mutable_messages(0) = call;

    auto reqResponse = execFuncWithPool(req);
    return req->messages(0);
}

void doWamrPoolExecution(faabric::Message& msg, int timeout = 1000)
{
    conf::FaasmConfig& conf = conf::getFaasmConfig();
    const std::string originalVm = conf.wasmVm;
    conf.wasmVm = "wamr";

    // Don't clean so that the WAMR configuration persists
    execFuncWithPool(msg, false, timeout);

    conf.wasmVm = originalVm;
}

faabric::Message execErrorFunction(faabric::Message& call)
{
    auto fac = std::make_shared<faaslet::FaasletFactory>();
    faabric::runner::FaabricMain m(fac);
    m.startRunner();

    // Convert message to request
    // TODO: do less hacky
    auto req = faabric::util::batchExecFactory(call.user(), call.function(), 1);
    req->set_appid(call.appid());
    *req->mutable_messages(0) = call;

    faabric::scheduler::Scheduler& sch = faabric::scheduler::getScheduler();
    sch.callFunctions(req);

    faabric::Message result = sch.getFunctionResult(call, 1);

    m.shutdown();

    return result;
}

void executeWithWamrPool(const std::string& user,
                         const std::string& func,
                         int timeout)
{
    faabric::Message call = faabric::util::messageFactory(user, func);
    doWamrPoolExecution(call, timeout);
}

void execSgxFunction(faabric::Message& call, const std::string& expectedOutput)
{
#ifndef FAASM_SGX_DISABLED_MODE
    conf::FaasmConfig& conf = conf::getFaasmConfig();
    const std::string originalVm = conf.wasmVm;
    conf.wasmVm = "sgx";

    wasm::EnclaveInterface enclaveInterface;
    enclaveInterface.bindToFunction(call);
    int returnValue = enclaveInterface.executeFunction(call);

    if (!expectedOutput.empty()) {
        std::string actualOutput = call.outputdata();
        REQUIRE(actualOutput == expectedOutput);
    }

    REQUIRE(returnValue == 0);
    REQUIRE(call.returnvalue() == 0);

    conf.wasmVm = originalVm;
#else
    SPDLOG_ERROR("Running SGX test but SGX is disabled");
#endif
}

void execFuncWithSgxPool(faabric::Message& call, int timeout)
{
#ifndef FAASM_SGX_DISABLED_MODE
    conf::FaasmConfig& conf = conf::getFaasmConfig();
    const std::string originalVm = conf.wasmVm;
    conf.wasmVm = "sgx";

    // Don't clean so that the SGX configuration persists
    execFuncWithPool(call, false, timeout);

    conf.wasmVm = originalVm;
#else
    SPDLOG_ERROR("Running SGX test but SGX is disabled");
#endif
}

void checkCallingFunctionGivesBoolOutput(const std::string& user,
                                         const std::string& funcName,
                                         bool expected)
{
    auto req = faabric::util::batchExecFactory(user, funcName, 1);
    auto& call = *req->mutable_messages(0);

    faabric::scheduler::Scheduler& sch = faabric::scheduler::getScheduler();
    sch.callFunctions(req);

    auto fac = std::make_shared<faaslet::FaasletFactory>();
    faabric::runner::FaabricMain m(fac);
    m.startRunner();

    // Check output is true
    faabric::Message result = sch.getFunctionResult(call, 1);
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

    m.shutdown();
}
}
