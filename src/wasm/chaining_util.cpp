#include <conf/FaasmConfig.h>
#include <faabric/executor/ExecutorContext.h>
#include <faabric/planner/PlannerClient.h>
#include <faabric/util/ExecGraph.h>
#include <faabric/util/batch.h>
#include <faabric/util/bytes.h>
#include <faabric/util/logging.h>
#include <wasm/WasmExecutionContext.h>
#include <wasm/WasmModule.h>
#include <wasm/chaining.h>

namespace wasm {
int awaitChainedCall(unsigned int messageId)
{
    int callTimeoutMs = conf::getFaasmConfig().chainedCallTimeout;
    auto* exec = faabric::executor::ExecutorContext::get()->getExecutor();

    int returnCode = 1;
    try {
        int appId = exec->getBoundMessage().appid();
        auto& plannerCli = faabric::planner::getPlannerClient();
        const faabric::Message result =
          plannerCli.getMessageResult(appId, messageId, callTimeoutMs);
        returnCode = result.returnvalue();
    } catch (faabric::executor::ChainedCallException& ex) {
        SPDLOG_ERROR(
          "Error getting chained call message: {}: {}", messageId, ex.what());
    } catch (faabric::redis::RedisNoResponseException& ex) {
        SPDLOG_ERROR("Timed out waiting for chained call: {}", messageId);
    } catch (std::exception& ex) {
        SPDLOG_ERROR("Non-timeout exception waiting for chained call: {}",
                     ex.what());
    }

    return returnCode;
}

int makeChainedCall(const std::string& functionName,
                    int wasmFuncPtr,
                    const char* pyFuncName,
                    const std::vector<uint8_t>& inputData)
{
    faabric::Message* originalCall =
      &faabric::executor::ExecutorContext::get()->getMsg();

    std::string user = originalCall->user();

    assert(!user.empty());
    assert(!functionName.empty());

    // Spawn a child batch exec (parent-child expressed by having the same app
    // id)
    std::shared_ptr<faabric::BatchExecuteRequest> req =
      faabric::util::batchExecFactory(originalCall->user(), functionName, 1);
    faabric::util::updateBatchExecAppId(req, originalCall->appid());

    // Propagate chaining-specific fields
    faabric::Message& msg = req->mutable_messages()->at(0);
    msg.set_inputdata(inputData.data(), inputData.size());
    msg.set_funcptr(wasmFuncPtr);

    // Propagate the command line if needed
    msg.set_cmdline(originalCall->cmdline());

    // Python properties
    msg.set_pythonuser(originalCall->pythonuser());
    msg.set_pythonfunction(originalCall->pythonfunction());
    if (pyFuncName != nullptr) {
        msg.set_pythonentry(pyFuncName);
    }
    msg.set_ispython(originalCall->ispython());

    if (originalCall->recordexecgraph()) {
        msg.set_recordexecgraph(true);
    }

    if (msg.funcptr() == 0) {
        SPDLOG_INFO("Chaining call {}/{} -> {}/{} (ids: {} -> {})",
                    originalCall->user(),
                    originalCall->function(),
                    msg.user(),
                    msg.function(),
                    originalCall->id(),
                    msg.id());
    } else {
        SPDLOG_INFO("Chaining nested call {}/{} (ids: {} -> {})",
                    msg.user(),
                    msg.function(),
                    originalCall->id(),
                    msg.id());
    }

    // Record the chained call in the executor before invoking the new
    // functions to avoid data races
    faabric::executor::ExecutorContext::get()->getExecutor()->addChainedMessage(
      req->messages(0));

    auto& plannerCli = faabric::planner::getPlannerClient();
    plannerCli.callFunctions(req);
    if (originalCall->recordexecgraph()) {
        faabric::util::logChainedFunction(*originalCall, msg);
    }

    return msg.id();
}

faabric::Message awaitChainedCallOutput(unsigned int messageId)
{
    int callTimeoutMs = conf::getFaasmConfig().chainedCallTimeout;
    auto* exec = faabric::executor::ExecutorContext::get()->getExecutor();

    faabric::Message result;
    try {
        auto msg = exec->getChainedMessage(messageId);
        auto& plannerCli = faabric::planner::getPlannerClient();
        result = plannerCli.getMessageResult(msg, callTimeoutMs);
    } catch (faabric::executor::ChainedCallException& e) {
        SPDLOG_ERROR(
          "Error awaiting for chained call {}: {}", messageId, e.what());
        throw std::runtime_error("Error awaiting for chained call");
    }

    if (result.type() == faabric::Message_MessageType_EMPTY) {
        SPDLOG_ERROR("Cannot find output for {}", messageId);
    }

    return result;
}
}
