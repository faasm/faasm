#include "WasmModule.h"
#include "syscalls.h"

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

    DEFINE_INTRINSIC_FUNCTION(env, "__faasm_await_call", void, __faasm_await_call, I32 messageId) {
        util::getLogger()->debug("S - await_call - {}", messageId);

        std::string resultKey = util::resultKeyFromMessageId(messageId);

        // TODO - await the call (using resultKey)
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__faasm_chain_function", void, __faasm_chain_function,
                              I32 namePtr, I32 inputDataPtr, I32 inputDataLen) {
        util::getLogger()->debug("S - chain_function - {} {} {}", namePtr, inputDataPtr, inputDataLen);

        message::Message *call = getExecutingCall();
        CallChain *callChain = getExecutingCallChain();
        std::string funcName = getStringFromWasm(namePtr);
        const std::vector<uint8_t> inputData = getBytesFromWasm(inputDataPtr, inputDataLen);

        // Add this to the chain of calls
        callChain->addCall(call->user(), funcName, inputData);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__faasm_chain_this", void, __faasm_chain_this,
                            I32 idx, I32 inputDataPtr, I32 inputDataLen) {
        util::getLogger()->debug("S - chain_this - {} {} {}", idx, inputDataPtr, inputDataLen);

        message::Message *call = getExecutingCall();
        CallChain *callChain = getExecutingCallChain();
        const std::vector<uint8_t> inputData = getBytesFromWasm(inputDataPtr, inputDataLen);

        // Add this to the chain of calls
        callChain->addCallThis(call->user(), call->function(), idx, inputData);
    }
}