#include "WAVMWasmModule.h"
#include "syscalls.h"

#include <scheduler/Scheduler.h>

#include <WAVM/Runtime/Runtime.h>
#include <WAVM/Runtime/Intrinsics.h>
#include <util/bytes.h>

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

        return awaitChainedCall(messageId);
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "__faasm_get_call_output", I32, __faasm_get_call_output, U32 messageId,
                                   I32 bufferPtr, I32 bufferLen) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->debug("S - get_call_output - {} {} {}", messageId, bufferPtr, bufferLen);

        auto buffer = &Runtime::memoryRef<uint8_t>(getExecutingModule()->defaultMemory, bufferPtr);

        // TODO - avoid always getting the function result even if we're just checking the length
        scheduler::GlobalMessageBus &globalBus = scheduler::getGlobalMessageBus();
        const message::Message result = globalBus.getFunctionResult(messageId, 0);

        if(bufferLen == 0) {
            return result.outputdata().size();
        }

        int outputLen = 0;
        if (result.type() == message::Message_MessageType_EMPTY) {
            logger->error("Cannot find output for {}", messageId);
        } else if (result.returnvalue() == 0) {
            std::vector<uint8_t> outputData = util::stringToBytes(result.outputdata());
            outputLen = util::safeCopyToBuffer(outputData, buffer, bufferLen);
        } else {
            logger->error("Cannot get output for {} (failed)", messageId);
        }

        return outputLen;
    }

    long __faasm_get_call_output(unsigned int messageId, unsigned char *buffer, long bufferLen) {
        throw std::runtime_error("Get call output not implemented in emulator");
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "__faasm_chain_function", U32, __faasm_chain_function,
                                   I32 namePtr, I32 inputDataPtr, I32 inputDataLen) {
        std::string funcName = getStringFromWasm(namePtr);
        util::getLogger()->debug("S - chain_function - {} {} {}", funcName, inputDataPtr, inputDataLen);

        const std::vector<uint8_t> inputData = getBytesFromWasm(inputDataPtr, inputDataLen);

        return makeChainedCall(funcName, 0, nullptr, inputData);
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "__faasm_chain_this", U32, __faasm_chain_this,
                                   I32 idx, I32 inputDataPtr, I32 inputDataLen) {
        util::getLogger()->debug("S - chain_this - {} {} {}", idx, inputDataPtr, inputDataLen);

        message::Message *call = getExecutingCall();
        const std::vector<uint8_t> inputData = getBytesFromWasm(inputDataPtr, inputDataLen);

        return makeChainedCall(call->function(), idx, nullptr, inputData);
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "__faasm_chain_py", U32, __faasm_chain_py,
                                   I32 namePtr, I32 inputDataPtr, I32 inputDataLen) {
        const std::string pyFuncName = getStringFromWasm(namePtr);
        util::getLogger()->debug("S - chain_py - {} {} {}", pyFuncName, inputDataPtr, inputDataLen);

        message::Message *call = getExecutingCall();
        const std::vector<uint8_t> inputData = getBytesFromWasm(inputDataPtr, inputDataLen);

        return makeChainedCall(call->function(), 0, pyFuncName.c_str(), inputData);
    }
}
