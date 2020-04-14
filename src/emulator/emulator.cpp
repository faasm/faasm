
extern "C" {
#include <faasm/core.h>
#include <faasm/host_interface.h>
#include <emulator/emulator_api.h>
}

#include <emulator/emulator.h>
#include <util/locks.h>
#include <util/logging.h>
#include <redis/Redis.h>
#include <state/State.h>
#include <thread>
#include <util/state.h>
#include <util/func.h>
#include <util/chaining.h>
#include <util/environment.h>
#include <util/json.h>
#include <util/bytes.h>
#include <scheduler/Scheduler.h>
#include <util/files.h>

/**
 * C++ emulation of Faasm system
 */

// Note thread-locality here
static thread_local message::Message _emulatedCall = message::Message();

static std::mutex threadsMutex;
static std::unordered_map<int, std::thread> threads;
static int threadCount = 1;

#define DUMMY_USER "emulated"

void resetEmulator() {
    _emulatedCall = message::Message();
    threads.clear();
    threadCount = 1;
}

std::vector<uint8_t> getEmulatorOutputData() {
    return util::stringToBytes(_emulatedCall.outputdata());
}

std::string getEmulatorOutputDataString() {
    const std::vector<uint8_t> outputData = getEmulatorOutputData();
    std::string outputStr;
    if (outputData.empty()) {
        outputStr = "Empty output";
    } else {
        outputStr = std::string(reinterpret_cast<const char *>(outputData.data()));
    }

    return outputStr;
}


std::string getEmulatorUser() {
    return _emulatedCall.user();
}

void setEmulatorUser(const char *newUser) {
    _emulatedCall.set_user(newUser);
}

char *emulatorGetAsyncResponse() {
    const std::string responseStr = util::buildAsyncResponse(_emulatedCall);
    char *responseData = new char[responseStr.size()];
    strcpy(responseData, responseStr.c_str());
    return responseData;
}

void emulatorSetCallStatus(bool success) {
    const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
    message::Message resultMsg = _emulatedCall;

    const std::string funcStr = util::funcToString(resultMsg, true);
    if (success) {
        logger->debug("Setting success status for {}", funcStr);
        resultMsg.set_returnvalue(0);
    } else {
        logger->debug("Setting failed status for {}", funcStr);
        resultMsg.set_returnvalue(1);
    }

    scheduler::GlobalMessageBus &globalBus = scheduler::getGlobalMessageBus();
    resultMsg.set_outputdata(getEmulatorOutputDataString());
    globalBus.setFunctionResult(resultMsg);
}

unsigned int setEmulatedMessageFromJson(const char *messageJson) {
    const message::Message msg = util::jsonToMessage(messageJson);
    return setEmulatedMessage(msg);
}

unsigned int setEmulatedMessage(const message::Message &msg) {
    _emulatedCall = msg;
    unsigned int msgId = util::setMessageId(_emulatedCall);

    const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
    const std::string funcStr = util::funcToString(_emulatedCall, true);
    logger->debug("Emulator set to {}", funcStr);

    return msgId;
}

std::shared_ptr<state::StateKeyValue> getKv(const char *key, size_t size) {
    state::State &s = state::getGlobalState();

    if (_emulatedCall.user().empty()) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->debug("Setting dummy emulator user {}", DUMMY_USER);
        setEmulatorUser(DUMMY_USER);
    }

    return s.getKV(_emulatedCall.user(), key, size);
}

void __faasm_write_output(const unsigned char *output, long outputLen) {
    util::getLogger()->debug("E - write_output {} {}", output, outputLen);
    _emulatedCall.set_outputdata(output, outputLen);
}


void __faasm_read_state(const char *key, unsigned char *buffer, long bufferLen) {
    util::getLogger()->debug("E - read_state {} {}", key, bufferLen);
    if (bufferLen == 0) {
        return;
    }

    auto kv = getKv(key, bufferLen);
    kv->get(buffer);
}

unsigned char *__faasm_read_state_ptr(const char *key, long totalLen) {
    util::getLogger()->debug("E - read_state_ptr {} {}", key, totalLen);
    auto kv = getKv(key, totalLen);
    return kv->get();
}

void __faasm_read_state_offset(const char *key, long totalLen, long offset, unsigned char *buffer, long bufferLen) {
    util::getLogger()->debug("E - read_state_offset {} {} {} {}", key, totalLen, offset, bufferLen);
    auto kv = getKv(key, totalLen);
    kv->getSegment(offset, buffer, bufferLen);
}

unsigned char *__faasm_read_state_offset_ptr(const char *key, long totalLen, long offset, long len) {
    util::getLogger()->debug("E - read_state_offset_ptr {} {} {} {}", key, totalLen, offset, len);
    auto kv = getKv(key, totalLen);
    return kv->getSegment(offset, len);
}

void __faasm_write_state(const char *key, const uint8_t *data, long dataLen) {
    util::getLogger()->debug("E - write_state {} {}", key, dataLen);
    auto kv = getKv(key, dataLen);
    kv->set(data);
}

void __faasm_append_state(const char *key, const uint8_t *data, long dataLen) {
    util::getLogger()->debug("E - append_state {} {}", key, dataLen);
    redis::Redis &redis = redis::Redis::getState();
    std::vector<uint8_t> bytes(data, data + dataLen);

    const std::string actualKey = util::keyForUser(getEmulatorUser(), key);
    redis.enqueueBytes(actualKey, bytes);
}

void __faasm_read_appended_state(const char *key, unsigned char *buffer, long bufferLen, long nElems) {
    util::getLogger()->debug("E - read_appended_state {} {} {}", key, bufferLen, nElems);

    // Read straight into buffer
    redis::Redis &redis = redis::Redis::getState();
    const std::string actualKey = util::keyForUser(getEmulatorUser(), key);

    redis.dequeueMultiple(actualKey, buffer, bufferLen, nElems);
}

void __faasm_clear_appended_state(const char *key) {
    util::getLogger()->debug("E - clear_appended_state {}", key);
    redis::Redis &redis = redis::Redis::getState();
    const std::string actualKey = util::keyForUser(getEmulatorUser(), key);
    redis.del(actualKey);
}

void __faasm_write_state_offset(const char *key, long totalLen, long offset, const unsigned char *data, long dataLen) {
    // Avoid excessive logging
    // util::getLogger()->debug("E - write_state_offset {} {} {} {}", key, totalLen, offset, dataLen);
    auto kv = getKv(key, totalLen);
    kv->setSegment(offset, data, dataLen);
}

unsigned int __faasm_write_state_from_file(const char *key, const char *filePath) {
    util::getLogger()->debug("E - write_state_from_file - {} {}", key, filePath);

    // Read file into bytes
    const std::vector<uint8_t> bytes = util::readFileToBytes(filePath);

    // Write to state
    const std::string actualKey = util::keyForUser(getEmulatorUser(), key);
    redis::Redis &redis = redis::Redis::getState();
    redis.set(actualKey, bytes);

    return bytes.size();
}

void __faasm_flag_state_dirty(const char *key, long totalLen) {
    // Avoid excessive logging
    // util::getLogger()->debug("E - flag_state_dirty {} {}", key, totalLen);
    auto kv = getKv(key, totalLen);
    kv->flagDirty();
}

void __faasm_flag_state_offset_dirty(const char *key, long totalLen, long offset, long dataLen) {
    // Avoid excessive logging
    // util::getLogger()->debug("E - flag_state_offset_dirty {} {} {} {}", key, totalLen, offset, dataLen);
    auto kv = getKv(key, totalLen);
    kv->flagSegmentDirty(offset, dataLen);
}


void __faasm_push_state_partial_mask(const char *key, const char *maskKey) {
    // Avoid excessive logging
    // util::getLogger()->debug("E - push_state_partial_mask {}", key, maskKey);
    auto kv = getKv(key, 0);
    auto maskKv = getKv(maskKey, 0);

    kv->pushPartialMask(maskKv);
}

void __faasm_push_state(const char *key) {
    util::getLogger()->debug("E - push_state {}", key);
    auto kv = getKv(key, 0);
    kv->pushFull();
}

void __faasm_push_state_partial(const char *key) {
    util::getLogger()->debug("E - push_state_partial {}", key);
    auto kv = getKv(key, 0);
    kv->pushPartial();
}

void __faasm_pull_state(const char *key, long stateLen) {
    util::getLogger()->debug("E - pull_state {}", key);
    auto kv = getKv(key, stateLen);
    kv->pull();
}

long __faasm_read_input(unsigned char *buffer, long bufferLen) {
    util::getLogger()->debug("E - read_input len {}", bufferLen);

    long inputLen;
    inputLen = _emulatedCall.inputdata().size();

    // This relies on thread-local _inputData
    if (bufferLen == 0) {
        return inputLen;
    }

    if (inputLen == 0) {
        return 0;
    }

    std::copy(_emulatedCall.inputdata().begin(), _emulatedCall.inputdata().end(), buffer);

    return bufferLen;
}

unsigned int _chain_local(int idx, int pyIdx, const unsigned char *buffer, long bufferLen) {
    util::getLogger()->debug("E - chain_this_local idx {} input len {}", idx, bufferLen);
    const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

    if (pyIdx > 0) {
        throw std::runtime_error("Not yet implemented");
    }

    // Create a fake thread ID for this thread
    int thisCallId;
    {
        util::UniqueLock threadsLock(threadsMutex);

        threadCount++;
        thisCallId = threadCount;

        // Spawn a thread to execute the function
        threads.emplace(std::pair<int, std::thread>(thisCallId, [idx, buffer, bufferLen] {
            // Set up input data for this thread (thread-local)
            _emulatedCall.set_inputdata(buffer, bufferLen);
            _emulatedCall.set_idx(idx);

            // Invoke the function
            _FaasmFuncPtr f = getFaasmFunc(idx);
            f();
        }));
    }

    logger->debug("Chained local function {} with call ID {}", idx, thisCallId);
    return thisCallId;
}

unsigned int _chain_knative(const std::string &funcName, int idx, int pyIdx, const unsigned char *buffer,
        long bufferLen) {
    const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
    logger->debug("E - chain_this_knative idx {} input len {}", idx, bufferLen);

    const std::string host = util::getEnvVar("FAASM_INVOKE_HOST", "");
    const std::string port = util::getEnvVar("FAASM_INVOKE_PORT", "");

    if (host.empty() || port.empty()) {
        logger->error("Expected FAASM_INVOKE_HOST and FAASM_INVOKE_PORT to be set");
        throw std::runtime_error("Missing invoke host and port");
    }

    int portInt = std::stoi(port);

    // Build the message to dispatch
    message::Message msg = util::messageFactory(_emulatedCall.user(), funcName);
    util::setMessageId(msg);

    msg.set_idx(idx);
    msg.set_inputdata(buffer, bufferLen);
    msg.set_ispython(_emulatedCall.ispython());
    msg.set_pythonuser(_emulatedCall.pythonuser());
    msg.set_pythonfunction(_emulatedCall.pythonfunction());
    msg.set_pythonidx(pyIdx);

    // Chained calls are always async and can be awaited by the caller
    msg.set_isasync(true);

    logger->debug("POST {}:{} ({})", host, portInt, util::messageToJson(msg));

    // Flush stdout before chaining
    fflush(stdout);

    // Make the call
    util::postJsonFunctionCall(host, portInt, msg);
    return msg.id();
}

unsigned int __faasm_chain_function(const char *name, const unsigned char *inputData, long inputDataSize) {
    util::SystemConfig &conf = util::getSystemConfig();
    if (conf.hostType == "knative") {
        return _chain_knative(name, 0, 0, inputData, inputDataSize);
    } else {
        throw std::runtime_error("Chaining by name not supported in emulator");
    }
}

unsigned int __faasm_chain_this(int idx, const unsigned char *buffer, long bufferLen) {
    util::SystemConfig &conf = util::getSystemConfig();
    if (conf.hostType == "knative") {
        return _chain_knative(_emulatedCall.function(), idx, 0, buffer, bufferLen);
    } else {
        return _chain_local(idx, 0, buffer, bufferLen);
    }
}

unsigned int __faasm_chain_py(int idx, const unsigned char *buffer, long bufferLen) {
    util::SystemConfig &conf = util::getSystemConfig();
    if (conf.hostType == "knative") {
        return _chain_knative(_emulatedCall.function(), 0, idx, buffer, bufferLen);
    } else {
        return _chain_local(0, idx, buffer, bufferLen);
    }
}

int _await_call_knative(unsigned int callId) {
    const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
    logger->debug("E - await_call_knative {}", callId);
    int timeoutMs = util::getSystemConfig().chainedCallTimeout;
    
    scheduler::GlobalMessageBus &bus = scheduler::getGlobalMessageBus();
    int returnCode = 1;
    try {
        const message::Message result = bus.getFunctionResult(callId, timeoutMs);
        returnCode = result.returnvalue();
    } catch (redis::RedisNoResponseException &ex) {
        logger->error("Timed out waiting for chained call: {}", callId);
    } catch (std::exception &ex) {
        logger->error("Non-timeout exception waiting for chained call: {}", ex.what());
    }

    logger->debug("Await returned {} for {}", returnCode, callId);

    return returnCode;
}

int _await_call_local(unsigned int callId) {
    const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
    logger->debug("E - await_call_local {}", callId);

    // Check this is valid
    if (threads.count(callId) == 0) {
        logger->error("Call with id {} doesn't exist", callId);
        throw std::runtime_error("Awaiting non-existent call");
    }

    // Join the thread to await its completion
    std::thread &t = threads[callId];
    if (t.joinable()) {
        t.join();
    } else {
        logger->error("Call with id {} not joinable", callId);
        throw std::runtime_error("Cannot join thread");
    }

    return 0;
}

int __faasm_await_call(unsigned int callId) {
    util::SystemConfig &conf = util::getSystemConfig();
    if (conf.hostType == "knative") {
        return _await_call_knative(callId);
    } else {
        return _await_call_local(callId);
    }
}

int __faasm_get_idx() {
    return _emulatedCall.idx();
}

int __faasm_get_py_idx() {
    return _emulatedCall.pythonidx();
}

void __faasm_lock_state_global(const char *key) {

}

void __faasm_unlock_state_global(const char *key) {

}

void __faasm_lock_state_read(const char *key) {

}

void __faasm_unlock_state_read(const char *key) {

}

void __faasm_lock_state_write(const char *key) {

}

void __faasm_unlock_state_write(const char *key) {

}

void copyStringToBuffer(unsigned char *buffer, const std::string &strIn) {
    ::strcpy(reinterpret_cast<char*>(buffer), strIn.c_str());
}

void __faasm_get_py_user(unsigned char *buffer, long bufferLen) {
    copyStringToBuffer(buffer, _emulatedCall.pythonuser());
}

void __faasm_get_py_func(unsigned char *buffer, long bufferLen) {
    copyStringToBuffer(buffer, _emulatedCall.pythonfunction());
}

unsigned int __faasm_conf_flag(const char *key) {
    return 0;
}