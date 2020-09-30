
extern "C"
{
#include <emulator/emulator_api.h>
#include <faasm/core.h>
#include <faasm/host_interface.h>
}

#include <emulator/emulator.h>
#include <faabric/scheduler/Scheduler.h>
#include <faabric/state/State.h>
#include <faabric/util/bytes.h>
#include <faabric/util/chaining.h>
#include <faabric/util/environment.h>
#include <faabric/util/files.h>
#include <faabric/util/func.h>
#include <faabric/util/json.h>
#include <faabric/util/locks.h>
#include <faabric/util/logging.h>
#include <faabric/util/state.h>
#include <thread>

/**
 * Used to emulate Faasm in native applications (i.e. anything that's not
 * WebAssembly)
 *
 * This will make use of the Faasm state abstractions where possible.
 */

// Note thread-locality here
static thread_local faabric::Message _emulatedCall = faabric::Message();
static thread_local faabric::state::State* _emulatedState = nullptr;

static std::mutex threadsMutex;
static std::unordered_map<int, std::thread> threads;
static int threadCount = 1;

#define DUMMY_USER "emulated"

// --------------------------------------------------------------
// EMULATOR-SPECIFIC
// --------------------------------------------------------------

void resetEmulator()
{
    _emulatedCall = faabric::Message();
    threads.clear();
    threadCount = 1;
}

std::vector<uint8_t> getEmulatorOutputData()
{
    return faabric::util::stringToBytes(_emulatedCall.outputdata());
}

std::string getEmulatorOutputDataString()
{
    const std::vector<uint8_t> outputData = getEmulatorOutputData();
    std::string outputStr;
    if (outputData.empty()) {
        outputStr = "Empty output";
    } else {
        outputStr =
          std::string(reinterpret_cast<const char*>(outputData.data()));
    }

    return outputStr;
}

std::string getEmulatorUser()
{
    return _emulatedCall.user();
}

void setEmulatorUser(const char* newUser)
{
    _emulatedCall.set_user(newUser);
}

faabric::state::State* getEmulatorState()
{
    if (_emulatedState == nullptr) {
        return &faabric::state::getGlobalState();
    } else {
        return _emulatedState;
    }
}

void setEmulatorState(faabric::state::State* state)
{
    _emulatedState = state;
}

void emulatorSetCallStatus(bool success)
{
    const std::shared_ptr<spdlog::logger>& logger = faabric::util::getLogger();
    faabric::Message resultMsg = _emulatedCall;

    const std::string funcStr = faabric::util::funcToString(resultMsg, true);

    if (success) {
        logger->debug("Setting success status for {}", funcStr);
        resultMsg.set_returnvalue(0);
    } else {
        logger->debug("Setting failed status for {}", funcStr);
        resultMsg.set_returnvalue(1);
    }

    faabric::scheduler::Scheduler& sch = faabric::scheduler::getScheduler();
    resultMsg.set_outputdata(getEmulatorOutputDataString());
    sch.setFunctionResult(resultMsg);
}

unsigned int setEmulatedMessageFromJson(const char* messageJson)
{
    const faabric::Message msg = faabric::util::jsonToMessage(messageJson);
    return setEmulatedMessage(msg);
}

unsigned int setEmulatedMessage(const faabric::Message& msg)
{
    _emulatedCall = msg;
    unsigned int msgId = faabric::util::setMessageId(_emulatedCall);

    const std::shared_ptr<spdlog::logger>& logger = faabric::util::getLogger();
    const std::string funcStr =
      faabric::util::funcToString(_emulatedCall, true);
    logger->debug("Emulator set to {}", funcStr);

    return msgId;
}

std::string getEmulatedUser()
{
    if (_emulatedCall.user().empty()) {
        const std::shared_ptr<spdlog::logger>& logger =
          faabric::util::getLogger();
        logger->debug("Setting dummy emulator user {}", DUMMY_USER);
        setEmulatorUser(DUMMY_USER);
    }

    return _emulatedCall.user();
}

// --------------------------------------------------------------
// FAASM HOST INTERFACE IMPLEMENTATION
// --------------------------------------------------------------

std::shared_ptr<faabric::state::StateKeyValue> getKv(const char* key,
                                                     size_t size)
{
    faabric::state::State* s = getEmulatorState();
    const std::string& emulatedUser = getEmulatedUser();
    return s->getKV(emulatedUser, key, size);
}

std::shared_ptr<faabric::state::StateKeyValue> getKv(const char* key)
{
    faabric::state::State* s = getEmulatorState();
    const std::string& emulatedUser = getEmulatedUser();
    return s->getKV(emulatedUser, key);
}

void __faasm_write_output(const unsigned char* output, long outputLen)
{
    faabric::util::getLogger()->debug(
      "E - write_output {} {}", output, outputLen);
    _emulatedCall.set_outputdata(output, outputLen);
}

long __faasm_read_state(const char* key, unsigned char* buffer, long bufferLen)
{
    faabric::util::getLogger()->debug("E - read_state {} {}", key, bufferLen);
    faabric::state::State* s = getEmulatorState();
    std::string emulatedUser = getEmulatedUser();

    if (bufferLen == 0) {
        return s->getStateSize(emulatedUser, key);
    }

    auto kv = getKv(key, bufferLen);
    kv->get(buffer);
    return kv->size();
}

unsigned char* __faasm_read_state_ptr(const char* key, long totalLen)
{
    faabric::util::getLogger()->debug(
      "E - read_state_ptr {} {}", key, totalLen);
    auto kv = getKv(key, totalLen);
    return kv->get();
}

void __faasm_read_state_offset(const char* key,
                               long totalLen,
                               long offset,
                               unsigned char* buffer,
                               long bufferLen)
{
    faabric::util::getLogger()->debug(
      "E - read_state_offset {} {} {} {}", key, totalLen, offset, bufferLen);
    auto kv = getKv(key, totalLen);
    kv->getChunk(offset, buffer, bufferLen);
}

unsigned char* __faasm_read_state_offset_ptr(const char* key,
                                             long totalLen,
                                             long offset,
                                             long len)
{
    faabric::util::getLogger()->debug(
      "E - read_state_offset_ptr {} {} {} {}", key, totalLen, offset, len);
    auto kv = getKv(key, totalLen);
    return kv->getChunk(offset, len);
}

void __faasm_write_state(const char* key, const uint8_t* data, long dataLen)
{
    faabric::util::getLogger()->debug("E - write_state {} {}", key, dataLen);
    auto kv = getKv(key, dataLen);
    kv->set(data);
}

void __faasm_append_state(const char* key, const uint8_t* data, long dataLen)
{
    faabric::util::getLogger()->debug("E - append_state {} {}", key, dataLen);
    auto kv = getKv(key);
    kv->append(data, dataLen);
}

void __faasm_read_appended_state(const char* key,
                                 unsigned char* buffer,
                                 long bufferLen,
                                 long nElems)
{
    faabric::util::getLogger()->debug(
      "E - read_appended_state {} {} {}", key, bufferLen, nElems);

    const std::string user = getEmulatorUser();
    auto kv = getKv(key);
    kv->getAppended(buffer, bufferLen, nElems);
}

void __faasm_clear_appended_state(const char* key)
{
    faabric::util::getLogger()->debug("E - clear_appended_state {}", key);
    auto kv = getKv(key);
    kv->clearAppended();
}

void __faasm_write_state_offset(const char* key,
                                long totalLen,
                                long offset,
                                const unsigned char* data,
                                long dataLen)
{
    // Avoid excessive logging
    // faabric::util::getLogger()->debug("E - write_state_offset {} {} {} {}",
    // key, totalLen, offset, dataLen);
    auto kv = getKv(key, totalLen);
    kv->setChunk(offset, data, dataLen);
}

unsigned int __faasm_write_state_from_file(const char* key,
                                           const char* filePath)
{
    faabric::util::getLogger()->debug(
      "E - write_state_from_file - {} {}", key, filePath);

    // Read file into bytes
    const std::vector<uint8_t> bytes = faabric::util::readFileToBytes(filePath);
    unsigned long fileLength = bytes.size();

    // Write to state
    faabric::state::State* s = getEmulatorState();
    auto kv = s->getKV(getEmulatorUser(), key, fileLength);
    kv->set(bytes.data());
    kv->pushFull();

    return bytes.size();
}

void __faasm_flag_state_dirty(const char* key, long totalLen)
{
    // Avoid excessive logging
    // faabric::util::getLogger()->debug("E - flag_state_dirty {} {}", key,
    // totalLen);
    auto kv = getKv(key, totalLen);
    kv->flagDirty();
}

void __faasm_flag_state_offset_dirty(const char* key,
                                     long totalLen,
                                     long offset,
                                     long dataLen)
{
    // Avoid excessive logging
    // faabric::util::getLogger()->debug("E - flag_state_offset_dirty {} {} {}
    // {}", key, totalLen, offset, dataLen);
    auto kv = getKv(key, totalLen);
    kv->flagChunkDirty(offset, dataLen);
}

void __faasm_push_state_partial_mask(const char* key, const char* maskKey)
{
    // Avoid excessive logging
    // faabric::util::getLogger()->debug("E - push_state_partial_mask {}", key,
    // maskKey);
    auto kv = getKv(key, 0);
    auto maskKv = getKv(maskKey, 0);

    kv->pushPartialMask(maskKv);
}

void __faasm_push_state(const char* key)
{
    faabric::util::getLogger()->debug("E - push_state {}", key);
    auto kv = getKv(key, 0);
    kv->pushFull();
}

void __faasm_push_state_partial(const char* key)
{
    faabric::util::getLogger()->debug("E - push_state_partial {}", key);
    auto kv = getKv(key, 0);
    kv->pushPartial();
}

void __faasm_pull_state(const char* key, long stateLen)
{
    faabric::util::getLogger()->debug("E - pull_state {}", key);
    auto kv = getKv(key, stateLen);
    kv->pull();
}

long __faasm_read_input(unsigned char* buffer, long bufferLen)
{
    faabric::util::getLogger()->debug("E - read_input len {}", bufferLen);

    long inputLen;
    inputLen = _emulatedCall.inputdata().size();

    // This relies on thread-local _inputData
    if (bufferLen == 0) {
        return inputLen;
    }

    if (inputLen == 0) {
        return 0;
    }

    std::copy(_emulatedCall.inputdata().begin(),
              _emulatedCall.inputdata().end(),
              buffer);

    return bufferLen;
}

unsigned int _chain_local(int idx,
                          const char* pyName,
                          const unsigned char* buffer,
                          long bufferLen)
{
    faabric::util::getLogger()->debug(
      "E - chain_this_local idx {} input len {}", idx, bufferLen);
    const std::shared_ptr<spdlog::logger>& logger = faabric::util::getLogger();

    if (pyName != nullptr) {
        throw std::runtime_error("Not yet implemented");
    }

    // Create a fake thread ID for this thread
    int thisCallId;
    {
        faabric::util::UniqueLock threadsLock(threadsMutex);

        threadCount++;
        thisCallId = threadCount;

        // Spawn a thread to execute the function
        threads.emplace(
          std::pair<int, std::thread>(thisCallId, [idx, buffer, bufferLen] {
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

unsigned int _chain_knative(const std::string& funcName,
                            int idx,
                            const char* pyName,
                            const unsigned char* buffer,
                            long bufferLen)
{
    const std::shared_ptr<spdlog::logger>& logger = faabric::util::getLogger();
    logger->debug(
      "E - chain_this_knative idx {} input length {}", idx, bufferLen);

    const std::string host = faabric::util::getEnvVar("FAASM_INVOKE_HOST", "");
    const std::string port = faabric::util::getEnvVar("FAASM_INVOKE_PORT", "");

    if (host.empty() || port.empty()) {
        logger->error(
          "Expected FAASM_INVOKE_HOST and FAASM_INVOKE_PORT to be set");
        throw std::runtime_error("Missing invoke host and port");
    }

    int portInt = std::stoi(port);

    // Build the message to dispatch
    faabric::Message msg =
      faabric::util::messageFactory(_emulatedCall.user(), funcName);
    msg.set_idx(idx);
    msg.set_inputdata(buffer, bufferLen);
    msg.set_ispython(_emulatedCall.ispython());
    msg.set_pythonuser(_emulatedCall.pythonuser());
    msg.set_pythonfunction(_emulatedCall.pythonfunction());

    if (pyName != nullptr) {
        msg.set_pythonentry(pyName);
    }

    // Chained calls are always async and can be awaited by the caller
    msg.set_isasync(true);

    std::string msgJson = faabric::util::messageToJson(msg);
    logger->debug("POST {}:{} ({})", host, portInt, msgJson);

    // Flush stdout before chaining
    fflush(stdout);

    // Make the call
    faabric::util::postJsonFunctionCall(host, portInt, msg);
    return msg.id();
}

unsigned int __faasm_chain_function(const char* name,
                                    const unsigned char* inputData,
                                    long inputDataSize)
{
    faabric::util::SystemConfig& conf = faabric::util::getSystemConfig();
    if (conf.hostType == "knative") {
        return _chain_knative(name, 0, 0, inputData, inputDataSize);
    } else {
        throw std::runtime_error("Chaining by name not supported in emulator");
    }
}

unsigned int __faasm_chain_this(int idx,
                                const unsigned char* buffer,
                                long bufferLen)
{
    faabric::util::SystemConfig& conf = faabric::util::getSystemConfig();
    if (conf.hostType == "knative") {
        return _chain_knative(
          _emulatedCall.function(), idx, nullptr, buffer, bufferLen);
    } else {
        return _chain_local(idx, nullptr, buffer, bufferLen);
    }
}

unsigned int __faasm_chain_py(const char* name,
                              const unsigned char* buffer,
                              long bufferLen)
{
    faabric::util::SystemConfig& conf = faabric::util::getSystemConfig();
    if (conf.hostType == "knative") {
        return _chain_knative(
          _emulatedCall.function(), 0, name, buffer, bufferLen);
    } else {
        return _chain_local(0, name, buffer, bufferLen);
    }
}

int _await_call_knative(unsigned int callId)
{
    const std::shared_ptr<spdlog::logger>& logger = faabric::util::getLogger();
    logger->debug("E - await_call_knative {}", callId);
    int timeoutMs = faabric::util::getSystemConfig().chainedCallTimeout;

    faabric::scheduler::Scheduler& sch = faabric::scheduler::getScheduler();
    int returnCode = 1;
    try {
        const faabric::Message result =
          sch.getFunctionResult(callId, timeoutMs);
        returnCode = result.returnvalue();
    } catch (faabric::redis::RedisNoResponseException& ex) {
        logger->error("Timed out waiting for chained call: {}", callId);
    } catch (std::exception& ex) {
        logger->error("Non-timeout exception waiting for chained call: {}",
                      ex.what());
    }

    logger->debug("Await returned {} for {}", returnCode, callId);

    return returnCode;
}

int _await_call_local(unsigned int callId)
{
    const std::shared_ptr<spdlog::logger>& logger = faabric::util::getLogger();
    logger->debug("E - await_call_local {}", callId);

    // Check this is valid
    if (threads.count(callId) == 0) {
        logger->error("Call with id {} doesn't exist", callId);
        throw std::runtime_error("Awaiting non-existent call");
    }

    // Join the thread to await its completion
    std::thread& t = threads[callId];
    if (t.joinable()) {
        t.join();
    } else {
        logger->error("Call with id {} not joinable", callId);
        throw std::runtime_error("Cannot join thread");
    }

    return 0;
}

int __faasm_await_call(unsigned int callId)
{
    faabric::util::SystemConfig& conf = faabric::util::getSystemConfig();
    if (conf.hostType == "knative") {
        return _await_call_knative(callId);
    } else {
        return _await_call_local(callId);
    }
}

int __faasm_await_call_output(unsigned int messageId,
                              unsigned char* buffer,
                              long bufferLen)
{
    throw std::runtime_error("Get call output not implemented in emulator");
}

int __faasm_get_idx()
{
    return _emulatedCall.idx();
}

void __faasm_lock_state_global(const char* key) {}

void __faasm_unlock_state_global(const char* key) {}

void __faasm_lock_state_read(const char* key) {}

void __faasm_unlock_state_read(const char* key) {}

void __faasm_lock_state_write(const char* key) {}

void __faasm_unlock_state_write(const char* key) {}

void copyStringToBuffer(unsigned char* buffer, const std::string& strIn)
{
    ::strcpy(reinterpret_cast<char*>(buffer), strIn.c_str());
}

void __faasm_get_py_user(unsigned char* buffer, long bufferLen)
{
    copyStringToBuffer(buffer, _emulatedCall.pythonuser());
}

void __faasm_get_py_func(unsigned char* buffer, long bufferLen)
{
    copyStringToBuffer(buffer, _emulatedCall.pythonfunction());
}

void __faasm_get_py_entry(unsigned char* buffer, long bufferLen)
{
    copyStringToBuffer(buffer, _emulatedCall.pythonentry());
}

unsigned int __faasm_conf_flag(const char* key)
{
    return 0;
}