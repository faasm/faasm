#include "WasmModule.h"
#include "syscalls.h"

#include <scheduler/Scheduler.h>

#include <WAVM/Runtime/Runtime.h>
#include <WAVM/Runtime/Intrinsics.h>

namespace wasm {
    void chainLink() {

    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "__faasm_get_idx", I32, __faasm_get_idx) {
        util::getLogger()->debug("S - get_idx");

        message::Message *call = getExecutingCall();
        int idx = call->idx();

        return idx;
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "__faasm_await_call", I32, __faasm_await_call, U32 messageId) {
        util::getLogger()->debug("S - await_call - {}", messageId);

        scheduler::GlobalMessageBus &bus = scheduler::getGlobalMessageBus();

        try {
            const message::Message result = bus.getFunctionResult(messageId, CHAINED_CALL_TIMEOUT);

            if (result.success()) {
                return 0;
            }
        } catch (redis::RedisNoResponseException &ex) {
            util::getLogger()->error("Timed out waiting for chained call: {}", messageId);
            return 1;
        } catch (std::exception &ex) {
            util::getLogger()->error("Non-timeout exception waiting for chained call: {}", ex.what());
            return 1;
        }

        return 1;
    }

    int _makeChainedCall(const std::string &functionName, int idx, const std::vector<uint8_t> &inputData) {
        scheduler::Scheduler &sch = scheduler::getScheduler();
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        message::Message *originalCall = getExecutingCall();

        // Chained calls should always be asynchronous as we don't sit around waiting for the result
        message::Message call = util::messageFactory(originalCall->user(), functionName);
        call.set_inputdata(inputData.data(), inputData.size());
        call.set_idx(idx);
        call.set_isasync(true);

        const std::string origStr = util::funcToString(*originalCall, false);
        const std::string chainedStr = util::funcToString(call, false);

        logger->debug("Chaining {} -> {}", origStr, chainedStr);
        sch.callFunction(call);

        return call.id();
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "__faasm_chain_function", U32, __faasm_chain_function,
                                   I32 namePtr, I32 inputDataPtr, I32 inputDataLen) {
        util::getLogger()->debug("S - chain_function - {} {} {}", namePtr, inputDataPtr, inputDataLen);

        std::string funcName = getStringFromWasm(namePtr);
        const std::vector<uint8_t> inputData = getBytesFromWasm(inputDataPtr, inputDataLen);

        return _makeChainedCall(funcName, 0, inputData);
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "__faasm_chain_this", U32, __faasm_chain_this,
                                   I32 idx, I32 inputDataPtr, I32 inputDataLen) {
        util::getLogger()->debug("S - chain_this - {} {} {}", idx, inputDataPtr, inputDataLen);

        message::Message *call = getExecutingCall();
        const std::vector<uint8_t> inputData = getBytesFromWasm(inputDataPtr, inputDataLen);

        return _makeChainedCall(call->function(), idx, inputData);
    }
}