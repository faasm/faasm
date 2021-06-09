#include "WasmModule.h"

#include <faabric/scheduler/Scheduler.h>
#include <faabric/util/bytes.h>
#include <faabric/util/func.h>
#include <faabric/util/logging.h>

#include <conf/FaasmConfig.h>
#include <wasm/chaining.h>

namespace wasm {
int awaitChainedCall(unsigned int messageId)
{
    int callTimeoutMs = conf::getFaasmConfig().chainedCallTimeout;

    int returnCode = 1;
    try {
        faabric::scheduler::Scheduler& sch = faabric::scheduler::getScheduler();
        const faabric::Message result =
          sch.getFunctionResult(messageId, callTimeoutMs);
        returnCode = result.returnvalue();
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
    faabric::scheduler::Scheduler& sch = faabric::scheduler::getScheduler();
    faabric::Message* originalCall = getExecutingCall();

    std::string user = originalCall->user();

    assert(!user.empty());
    assert(!functionName.empty());

    SPDLOG_DEBUG(
      "Chaining call {}/{}:{}", user, functionName, originalCall->id());

    std::shared_ptr<faabric::BatchExecuteRequest> req =
      faabric::util::batchExecFactory(originalCall->user(), functionName, 1);

    faabric::Message& msg = req->mutable_messages()->at(0);
    msg.set_inputdata(inputData.data(), inputData.size());
    msg.set_funcptr(wasmFuncPtr);

    msg.set_pythonuser(originalCall->pythonuser());
    msg.set_pythonfunction(originalCall->pythonfunction());
    if (pyFuncName != nullptr) {
        msg.set_pythonentry(pyFuncName);
    }
    msg.set_ispython(originalCall->ispython());

    sch.callFunctions(req);
    sch.logChainedFunction(originalCall->id(), msg.id());

    return msg.id();
}

int spawnChainedThread(const std::string& snapshotKey,
                       size_t snapshotSize,
                       int funcPtr,
                       int argsPtr)
{
    faabric::scheduler::Scheduler& sch = faabric::scheduler::getScheduler();

    faabric::Message* originalCall = getExecutingCall();
    faabric::Message call = faabric::util::messageFactory(
      originalCall->user(), originalCall->function());
    call.set_isasync(true);

    // Snapshot details
    call.set_snapshotkey(snapshotKey);

    // Function pointer and args
    // NOTE - with a pthread interface we only ever pass the function a single
    // pointer argument, hence we use the input data here to hold this argument
    // as a string
    call.set_funcptr(funcPtr);
    call.set_inputdata(std::to_string(argsPtr));

    const std::string origStr =
      faabric::util::funcToString(*originalCall, false);
    const std::string chainedStr = faabric::util::funcToString(call, false);

    // Schedule the call
    sch.callFunction(call);

    return call.id();
}

int awaitChainedCallOutput(unsigned int messageId,
                           uint8_t* buffer,
                           int bufferLen)
{

    int callTimeoutMs = conf::getFaasmConfig().chainedCallTimeout;

    faabric::scheduler::Scheduler& sch = faabric::scheduler::getScheduler();
    const faabric::Message result =
      sch.getFunctionResult(messageId, callTimeoutMs);

    if (result.type() == faabric::Message_MessageType_EMPTY) {
        SPDLOG_ERROR("Cannot find output for {}", messageId);
    }

    std::vector<uint8_t> outputData =
      faabric::util::stringToBytes(result.outputdata());
    int outputLen =
      faabric::util::safeCopyToBuffer(outputData, buffer, bufferLen);

    if (outputLen < outputData.size()) {
        SPDLOG_WARN(
          "Undersized output buffer: {} for {} output", bufferLen, outputLen);
    }

    return result.returnvalue();
}
}
