#include "WasmModule.h"

#include <scheduler/Scheduler.h>
#include <util/bytes.h>


namespace wasm {
    int awaitChainedCall(unsigned int messageId) {
        int callTimeoutMs = util::getSystemConfig().chainedCallTimeout;

        int returnCode = 1;
        try {
            scheduler::Scheduler &sch = scheduler::getScheduler();
            const message::Message result = sch.getFunctionResult(messageId, callTimeoutMs);
            returnCode = result.returnvalue();
        } catch (redis::RedisNoResponseException &ex) {
            util::getLogger()->error("Timed out waiting for chained call: {}", messageId);
        } catch (std::exception &ex) {
            util::getLogger()->error("Non-timeout exception waiting for chained call: {}", ex.what());
        }

        return returnCode;
    }

    int makeChainedCall(const std::string &functionName, int idx, const char *pyFuncName,
                        const std::vector<uint8_t> &inputData) {
        scheduler::Scheduler &sch = scheduler::getScheduler();
        message::Message *originalCall = getExecutingCall();

        // Chained calls should be asynchronous as we will wait for the result on the message queue
        message::Message call = util::messageFactory(originalCall->user(), functionName);
        call.set_inputdata(inputData.data(), inputData.size());
        call.set_idx(idx);
        call.set_isasync(true);

        call.set_pythonuser(originalCall->pythonuser());
        call.set_pythonfunction(originalCall->pythonfunction());
        if (pyFuncName != nullptr) {
            call.set_pythonentry(pyFuncName);
        }
        call.set_ispython(originalCall->ispython());

        const std::string origStr = util::funcToString(*originalCall, false);
        const std::string chainedStr = util::funcToString(call, false);

        util::SystemConfig &conf = util::getSystemConfig();
        sch.callFunction(call);
        util::getLogger()->debug("Chained {} ({}) -> {} ({})",
                                 origStr,
                                 conf.endpointHost,
                                 chainedStr,
                                 call.scheduledhost()
        );

        // Check if we need to log this
        if(conf.execGraphMode == "on") {
            sch.logChainedFunction(originalCall->id(), call.id());
        }
        
        return call.id();
    }

    int spawnChainedThread(const std::string &snapshotKey, size_t snapshotSize, int funcPtr, int argsPtr) {
        scheduler::Scheduler &sch = scheduler::getScheduler();

        message::Message *originalCall = getExecutingCall();
        message::Message call = util::messageFactory(originalCall->user(), originalCall->function());
        call.set_isasync(true);

        // Snapshot details
        call.set_snapshotkey(snapshotKey);
        call.set_snapshotsize(snapshotSize);

        // Function pointer and args
        // NOTE - with a pthread interface we only ever pass the function a single pointer argument,
        // hence we use the input data here to hold this argument as a string
        call.set_funcptr(funcPtr);
        call.set_inputdata(std::to_string(argsPtr));

        const std::string origStr = util::funcToString(*originalCall, false);
        const std::string chainedStr = util::funcToString(call, false);

        // Schedule the call
        sch.callFunction(call);
        util::getLogger()->debug("Chained thread {} ({}) -> {} {}({}) ({})",
                                 origStr,
                                 util::getSystemConfig().endpointHost,
                                 chainedStr,
                                 funcPtr, argsPtr, call.scheduledhost()
        );

        return call.id();
    }

    int awaitChainedCallOutput(unsigned int messageId, uint8_t *buffer, int bufferLen) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        int callTimeoutMs = util::getSystemConfig().chainedCallTimeout;

        scheduler::Scheduler &sch = scheduler::getScheduler();
        const message::Message result = sch.getFunctionResult(messageId, callTimeoutMs);

        if (result.type() == message::Message_MessageType_EMPTY) {
            logger->error("Cannot find output for {}", messageId);
        }

        std::vector<uint8_t> outputData = util::stringToBytes(result.outputdata());
        int outputLen = util::safeCopyToBuffer(outputData, buffer, bufferLen);

        if (outputLen < outputData.size()) {
            logger->warn("Undersized output buffer: {} for {} output", bufferLen, outputLen);
        }

        return result.returnvalue();
    }
}