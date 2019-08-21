#include "WasmModule.h"
#include "syscalls.h"

#include <scheduler/Scheduler.h>

#include <WAVM/Runtime/Runtime.h>
#include <WAVM/Runtime/RuntimeData.h>
#include <WAVM/Runtime/Intrinsics.h>

namespace wasm {
    void chainLink() {

    }

    DEFINE_INTRINSIC_FUNCTION(env, "__faasm_get_idx", I32, __faasm_get_idx) {
        util::getLogger()->debug("S - get_idx");

        message::Message *call = getExecutingCall();
        int idx = call->idx();

        return idx;
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__faasm_await_call", I32, __faasm_await_call, U32 messageId) {
        util::getLogger()->debug("S - await_call - {}", messageId);

        scheduler::GlobalMessageBus &bus = scheduler::getGlobalMessageBus();
        const message::Message &result = bus.getFunctionResult(messageId, CHAINED_CALL_TIMEOUT);

        if(result.success()) {
            return 0;
        } else {
            return 1;
        }
    }

    int _makeChainedCall(std::string functionName, int idx, const std::vector<uint8_t> &inputData) {
        scheduler::Scheduler &sch = scheduler::getScheduler();
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        message::Message *originalCall = getExecutingCall();

        message::Message call = util::messageFactory(originalCall->user(), functionName);
        call.set_inputdata(inputData.data(), inputData.size());

        const std::string origStr = util::funcToString(*originalCall, false);

        util::setMessageIdx(call, idx);
        const std::string chainedStr = util::funcToString(call, false);

        // TODO: Avoid this approach to recursive calls, add some option of delay in the scheduler?
        // Wait a bit before making a recursive call
        if (originalCall->function() == call.function() && idx == 0) {
            logger->debug("Delaying chained call {} -> {}", origStr, chainedStr);
            uint microseconds = (uint) 500 * 1000; // 500ms
            usleep(microseconds);
        }

        logger->debug("Chaining {} -> {}", origStr, chainedStr);
        sch.callFunction(call);

        return call.id();
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__faasm_chain_function", U32, __faasm_chain_function,
                              I32 namePtr, I32 inputDataPtr, I32 inputDataLen) {
        util::getLogger()->debug("S - chain_function - {} {} {}", namePtr, inputDataPtr, inputDataLen);

        std::string funcName = getStringFromWasm(namePtr);
        const std::vector<uint8_t> inputData = getBytesFromWasm(inputDataPtr, inputDataLen);

        return _makeChainedCall(funcName, 0, inputData);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__faasm_chain_this", U32, __faasm_chain_this,
                              I32 idx, I32 inputDataPtr, I32 inputDataLen) {
        util::getLogger()->debug("S - chain_this - {} {} {}", idx, inputDataPtr, inputDataLen);

        message::Message *call = getExecutingCall();
        const std::vector<uint8_t> inputData = getBytesFromWasm(inputDataPtr, inputDataLen);

        return _makeChainedCall(call->function(), idx, inputData);
    }
}