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

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "__faasm_get_py_idx", I32, __faasm_get_py_idx) {
        util::getLogger()->debug("S - get_py_idx");

        message::Message *call = getExecutingCall();
        int idx = call->pythonidx();

        return idx;
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "__faasm_await_call", I32, __faasm_await_call, U32 messageId) {
        util::getLogger()->debug("S - await_call - {}", messageId);

        scheduler::GlobalMessageBus &bus = scheduler::getGlobalMessageBus();
        scheduler::Scheduler &scheduler = scheduler::getScheduler();
        
        // Free this thread
        message::Message *msg = getExecutingCall();
        scheduler.notifyAwaiting(*msg);

        int returnCode = 1;
        try {
            const message::Message result = bus.getFunctionResult(messageId, CHAINED_CALL_TIMEOUT);

            if (result.success()) {
                returnCode = 0;
            }
        } catch (redis::RedisNoResponseException &ex) {
            util::getLogger()->error("Timed out waiting for chained call: {}", messageId);
        } catch (std::exception &ex) {
            util::getLogger()->error("Non-timeout exception waiting for chained call: {}", ex.what());
        }

        scheduler.notifyFinishedAwaiting(*msg);

        return returnCode;
    }

    int _makeChainedCall(const std::string &functionName, int idx, int pyIdx, const std::vector<uint8_t> &inputData) {
        scheduler::Scheduler &sch = scheduler::getScheduler();
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        message::Message *originalCall = getExecutingCall();

        // Chained calls should be asynchronous as we will wait for the result on the message queue
        message::Message call = util::messageFactory(originalCall->user(), functionName);
        call.set_inputdata(inputData.data(), inputData.size());
        call.set_idx(idx);
        call.set_isasync(true);
        call.set_pythonuser(originalCall->pythonuser());
        call.set_pythonfunction(originalCall->pythonfunction());
        call.set_pythonidx(pyIdx);

        const std::string origStr = util::funcToString(*originalCall, false);
        const std::string chainedStr = util::funcToString(call, false);

        sch.callFunction(call);
        logger->debug("Chained {} ({}) -> {} ({})", origStr, util::getNodeId(), chainedStr, call.schedulednode());

        return call.id();
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "__faasm_chain_this", U32, __faasm_chain_this,
                                   I32 idx, I32 inputDataPtr, I32 inputDataLen) {
        util::getLogger()->debug("S - chain_this - {} {} {}", idx, inputDataPtr, inputDataLen);

        message::Message *call = getExecutingCall();
        const std::vector<uint8_t> inputData = getBytesFromWasm(inputDataPtr, inputDataLen);

        return _makeChainedCall(call->function(), idx, 0, inputData);
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "__faasm_chain_py", U32, __faasm_chain_py,
                                   I32 idx, I32 inputDataPtr, I32 inputDataLen) {
        util::getLogger()->debug("S - chain_py - {} {} {}", idx, inputDataPtr, inputDataLen);

        message::Message *call = getExecutingCall();
        const std::vector<uint8_t> inputData = getBytesFromWasm(inputDataPtr, inputDataLen);

        return _makeChainedCall(call->function(), 0, idx, inputData);
    }
}