
extern "C" {
#include <faasm/core.h>
#include <faasm/host_interface.h>
#include <emulator/emulator_api.h>
}

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

/**
 * C++ emulation of Faasm system
 */

// We use an empty emulator user by default as it's easier to reason about keys in state
static std::string _user;
static std::string _function;
static std::string _pythonUser;
static std::string _pythonFunction;

// Note thread locality here to handle multiple locally chained functions
static thread_local std::vector<uint8_t> _threadLocalInputData;
static thread_local std::vector<uint8_t> _threadLocalOutputData;
static thread_local int _threadLocalfuncIdx = 0;
static thread_local int _threadLocalPyIdx = 0;

// Note thread locality here to handle multiple locally chained functions
static std::vector<uint8_t> _inputData;
static std::vector<uint8_t> _outputData;
static int _funcIdx = 0;
static int _pyIdx = 0;

static std::mutex threadsMutex;
static std::unordered_map<int, std::thread> threads;
static int threadCount = 1;

#define DUMMY_USER "emulated"

void resetEmulator() {
    _outputData.clear();
    _inputData.clear();
    _threadLocalOutputData.clear();
    _threadLocalInputData.clear();

    _funcIdx = 0;
    _threadLocalfuncIdx = 0;

    _function = "";

    threads.clear();
    threadCount = 1;
}

bool noThreadLocal() {
    util::SystemConfig &conf = util::getSystemConfig();
    return conf.hostType == "knative";
}

std::vector<uint8_t> getEmulatorOutputData() {
    if (noThreadLocal()) {
        return _outputData;
    } else {
        return _threadLocalOutputData;
    }
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

void setEmulatorInputData(const std::vector<uint8_t> &inputIn) {
    _inputData = inputIn;
    _threadLocalInputData = inputIn;
}

void setEmulatorFunctionIdx(int idx) {
    _funcIdx = idx;
    _threadLocalfuncIdx = idx;
}

std::string getEmulatorUser() {
    return _user;
}

void setEmulatorUser(const char *newUser) {
    _user = newUser;
}

char *emulatorGetCallStatus(unsigned int messageId) {
    // Get message with no delay
    scheduler::GlobalMessageBus &globalBus = scheduler::getGlobalMessageBus();
    const message::Message result = globalBus.getFunctionResult(messageId, 0);

    char *strResult = new char[20];
    if (result.type() == message::Message_MessageType_EMPTY) {
        sprintf(strResult, "RUNNING");
    } else if (result.success()) {
        sprintf(strResult, "SUCCESS");
    } else {
        sprintf(strResult, "FAILED");
    }

    return strResult;
}

void emulatorSetCallStatus(unsigned int messageId, bool success) {
    scheduler::GlobalMessageBus &globalBus = scheduler::getGlobalMessageBus();
    const message::Message tempMsg = util::messageFactory(getEmulatorUser(), _function);
    
    globalBus.setFunctionResult(tempMsg, success);
}


std::string getEmulatorFunction() {
    return _function;
}

void setEmulatorFunction(const char *newFunction) {
    _function = newFunction;
}

void unsetEmulatorUser() {
    _user = "";
}

std::string getEmulatorPythonUser() {
    return _pythonUser;
}

void setEmulatorPythonUser(const char *newUser) {
    _pythonUser = newUser;
}

void setEmulatorPyIdx(int idx) {
    _pyIdx = idx;
    _threadLocalPyIdx = idx;
}

std::string getEmulatorPythonFunction() {
    return _pythonFunction;
}

void setEmulatorPythonFunction(const char *newFunction) {
    _pythonFunction = newFunction;
}


void setEmulatedMessage(const char *messageJson) {
    const message::Message msg = util::jsonToMessage(messageJson);

    setEmulatorUser(msg.user().c_str());
    setEmulatorFunction(msg.function().c_str());
    setEmulatorFunctionIdx(msg.idx());

    setEmulatorInputData(util::stringToBytes(msg.inputdata()));

    setEmulatorPythonUser(msg.pythonuser().c_str());
    setEmulatorPythonFunction(msg.pythonfunction().c_str());
    setEmulatorPyIdx(msg.pythonidx());
}


std::shared_ptr<state::StateKeyValue> getKv(const char *key, size_t size) {
    state::State &s = state::getGlobalState();

    if (_user.empty()) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->debug("Setting dummy emulator user {}", DUMMY_USER);
        setEmulatorUser(DUMMY_USER);
    }

    return s.getKV(_user, key, size);
}

void __faasm_write_output(const unsigned char *output, long outputLen) {
    util::getLogger()->debug("E - write_output {} {}", output, outputLen);
    const std::vector<uint8_t> outputVec = std::vector<uint8_t>(output, output + outputLen);

    _outputData = outputVec;
    _threadLocalOutputData = outputVec;
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
    if (noThreadLocal()) {
        inputLen = _inputData.size();
    } else {
        inputLen = _threadLocalInputData.size();
    }

    // This relies on thread-local _inputData
    if (bufferLen == 0) {
        return inputLen;
    }

    if (inputLen == 0) {
        return 0;
    }

    if (noThreadLocal()) {
        std::copy(_inputData.begin(), _inputData.end(), buffer);
    } else {
        std::copy(_threadLocalInputData.begin(), _threadLocalInputData.end(), buffer);
    }

    return bufferLen;
}

unsigned int _chain_local(int idx, int pyIdx, const unsigned char *buffer, long bufferLen) {
    util::getLogger()->debug("E - chain_this_local idx {} input len {}", idx, bufferLen);
    const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

    if (pyIdx > 0) {
        throw std::runtime_error("Not yet implemented");
    }

    // Create vector to hold inputs
    std::vector<uint8_t> inputs(buffer, buffer + bufferLen);

    // Create a fake thread ID for this thread
    int thisCallId;
    {
        util::UniqueLock threadsLock(threadsMutex);

        threadCount++;
        thisCallId = threadCount;

        // Spawn a thread to execute the function
        threads.emplace(std::pair<int, std::thread>(thisCallId, [idx, inputs] {
            // Set up input data for this thread (thread-local)
            setEmulatorInputData(inputs);
            setEmulatorFunctionIdx(idx);

            // Invoke the function
            _FaasmFuncPtr f = getFaasmFunc(idx);
            f();
        }));
    }

    logger->debug("Chained local function {} with call ID {}", idx, thisCallId);
    return thisCallId;
}

unsigned int _chain_knative(int idx, int pyIdx, const unsigned char *buffer, long bufferLen) {
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
    message::Message msg = util::messageFactory(_user, _function);
    msg.set_idx(idx);
    msg.set_inputdata(buffer, bufferLen);
    msg.set_pythonuser(_pythonUser);
    msg.set_pythonfunction(_pythonFunction);
    msg.set_pythonidx(pyIdx);

    // We will be awaiting the response in a thread in the background, therefore this must _not_ be async
    msg.set_isasync(false);

    logger->debug("Invoking endpoint at {}:{}", host, portInt);
    logger->debug("Invoking JSON : {}", util::messageToJson(msg));

    // Flush stdout before chaining
    fflush(stdout);

    // Make the call in a thread in the background so we can continue
    {
        util::UniqueLock threadsLock(threadsMutex);

        // Spawn a thread to execute the function
        // Use the message ID as the key so we can join it when awaiting
        threads.emplace(std::pair<int, std::thread>(msg.id(), [host, portInt, msg] {

            util::postJsonFunctionCall(host, portInt, msg);
        }));
    }

    const std::string funcStr = util::funcToString(msg, true);
    logger->debug("Requested knative function {}", funcStr);

    return msg.id();
}

unsigned int __faasm_chain_this(int idx, const unsigned char *buffer, long bufferLen) {
    util::SystemConfig &conf = util::getSystemConfig();
    if (conf.hostType == "knative") {
        return _chain_knative(idx, 0, buffer, bufferLen);
    } else {
        return _chain_local(idx, 0, buffer, bufferLen);
    }
}

unsigned int __faasm_chain_py(int idx, const unsigned char *buffer, long bufferLen) {
    util::SystemConfig &conf = util::getSystemConfig();
    if (conf.hostType == "knative") {
        return _chain_knative(0, idx, buffer, bufferLen);
    } else {
        return _chain_local(0, idx, buffer, bufferLen);
    }
}

int __faasm_await_call(unsigned int callId) {
    const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
    logger->debug("E - await_call {}", callId);

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

int __faasm_get_idx() {
    // Relies on thread-local idx
    if (noThreadLocal()) {
        return _funcIdx;
    } else {
        return _threadLocalfuncIdx;
    }
}

int __faasm_get_py_idx() {
    if (noThreadLocal()) {
        return _pyIdx;
    } else {
        return _threadLocalPyIdx;
    }
}

void __faasm_lock_state_read(const char *key) {

}

void __faasm_unlock_state_read(const char *key) {

}

void __faasm_lock_state_write(const char *key) {

}

void __faasm_unlock_state_write(const char *key) {

}

void __faasm_get_py_user(unsigned char *buffer, long bufferLen) {
    const std::vector<uint8_t> bytes = util::stringToBytes(_pythonUser);
    std::copy(bytes.data(), bytes.data() + bytes.size(), buffer);
}

void __faasm_get_py_func(unsigned char *buffer, long bufferLen) {
    const std::vector<uint8_t> bytes = util::stringToBytes(_pythonFunction);
    std::copy(bytes.data(), bytes.data() + bytes.size(), buffer);
}