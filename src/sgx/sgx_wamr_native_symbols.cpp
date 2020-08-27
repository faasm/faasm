#include <cstring>

#include <sgx/SGXWAMRWasmModule.h>

using namespace faabric::state;

#define STATE_KV(key, stateLen) State &state = getGlobalState(); \
    std::shared_ptr<StateKeyValue> stateKv = state.getKV(wasm::getExecutingCall()->user(), \
                                                       std::string(key), \
                                                       stateLen);

namespace wasm {
    extern int awaitChainedCall(unsigned int messageId);

    extern int awaitChainedCallOutput(unsigned int messageId, uint8_t *buffer, int bufferLen);

    extern int makeChainedCall(const std::string &functionName, int idx, const char *pyFuncName,
                               const std::vector<uint8_t> &inputData);
}

extern "C" {

uint64_t ocall_faasm_read_state_size(const char *key) {
    State &state = getGlobalState();
    return (uint64_t) state.getStateSize(wasm::getExecutingCall()->user(), std::string(key));
}

uint64_t ocall_faasm_read_state(const char *key, uint8_t *buffer_ptr, const uint32_t buffer_len) {
    STATE_KV(key, buffer_len)
    stateKv->get(buffer_ptr);
    return stateKv->size();
}

void ocall_faasm_read_appended_state(const char *key, uint8_t *buffer_ptr, const uint32_t buffer_len,
                                     const uint32_t element_num) {
    STATE_KV(key, buffer_len)
    stateKv->getAppended(buffer_ptr, buffer_len, element_num);
}

void ocall_faasm_read_state_offset(const char *key, const uint64_t total_len,
                                   const uint64_t offset, uint8_t *buffer_ptr, const uint32_t buffer_len) {
    STATE_KV(key, total_len)
    stateKv->getChunk(offset, buffer_ptr, buffer_len);
}

void ocall_faasm_write_state(const char *key, const uint8_t *buffer_ptr, const uint32_t buffer_len) {
    STATE_KV(key, buffer_len)
    stateKv->set(buffer_ptr);
}

void ocall_faasm_append_state(const char *key, const uint8_t *buffer_ptr, const uint32_t buffer_len) {
    STATE_KV(key, buffer_len)
    stateKv->append(buffer_ptr, buffer_len);
}

void ocall_faasm_clear_appended_state(const char *key) {
    STATE_KV(key, 0)
    stateKv->clearAppended();
}

void ocall_faasm_write_state_offset(const char *key, const uint64_t total_len, const uint64_t offset,
                                    const uint8_t *buffer_ptr, const uint32_t buffer_len) {
    STATE_KV(key, total_len)
    stateKv->setChunk(offset, buffer_ptr, buffer_len);
}

void ocall_faasm_flag_state_dirty(const char *key, const uint64_t total_len) {
    STATE_KV(key, total_len)
    stateKv->flagDirty();
}

void ocall_faasm_flag_state_offset_dirty(const char *key, const uint64_t total_len, const uint64_t offset,
                                         const uint64_t len) {
    STATE_KV(key, total_len)
    stateKv->flagChunkDirty(offset, len);
}

void ocall_faasm_push_state(const char *key) {
    STATE_KV(key, 0)
    stateKv->pushFull();
}

void ocall_faasm_push_state_partial(const char *key) {
    STATE_KV(key, 0)
    stateKv->pushPartial();
}

void ocall_faasm_push_state_partial_mask(const char *key, const char *mask_key) {
    STATE_KV(key, 0)
    auto maskKv = state.getKV(wasm::getExecutingCall()->user(), std::string(mask_key), 0);
    stateKv->pushPartialMask(maskKv);
}

void ocall_faasm_pull_state(const char *key, const uint64_t state_len) {
    STATE_KV(key, state_len)
    stateKv->pull();
}

void ocall_faasm_lock_state_global(const char *key) {
    STATE_KV(key, 0)
    stateKv->lockGlobal();
}

void ocall_faasm_unlock_state_global(const char *key) {
    STATE_KV(key, 0)
    stateKv->unlockGlobal();
}

void ocall_faasm_lock_state_read(const char *key) {
    STATE_KV(key, 0)
    stateKv->lockRead();
}

void ocall_faasm_unlock_state_read(const char *key) {
    STATE_KV(key, 0)
    stateKv->unlockRead();
}

void ocall_faasm_lock_state_write(const char *key) {
    STATE_KV(key, 0)
    stateKv->lockWrite();
}

void ocall_faasm_unlock_state_write(const char *key) {
    STATE_KV(key, 0)
    stateKv->unlockWrite();
}

unsigned int ocall_faasm_get_input_size(void) {
    return wasm::getExecutingCall()->inputdata().size();
}

void ocall_faasm_get_input(uint8_t *buffer, unsigned int buffer_size) {
    faabric::Message *msg = wasm::getExecutingCall();

    if (msg->inputdata().size() > 0) {
        const std::string &_input = msg->inputdata();
        if (_input.size() > buffer_size) {
            memcpy(buffer, _input.data(), buffer_size);
        } else {
            memcpy(buffer, _input.data(), _input.size());
        }
    }
}

void ocall_faasm_set_output(uint8_t *output, unsigned int output_size) {
    wasm::getExecutingCall()->set_outputdata((void *) output, output_size);
}

unsigned int ocall_faasm_chain_function_input(const char *name, const uint8_t *input, unsigned int input_size) {
    std::vector<uint8_t> _input(input, input + input_size);
    return wasm::makeChainedCall(std::string(name), 0, nullptr, _input);
}

unsigned int ocall_faasm_chain_this_input(const int idx, uint8_t *input, unsigned int input_size) {
    const std::vector<uint8_t> _input(input, input + input_size);
    return wasm::makeChainedCall(wasm::getExecutingCall()->function(), idx, nullptr, _input);
}

unsigned int ocall_faasm_await_call(unsigned int call_id) {
    return wasm::awaitChainedCall(call_id);
}

unsigned int ocall_faasm_await_call_output(unsigned int call_id, uint8_t *buffer, unsigned int buffer_size) {
    return wasm::awaitChainedCallOutput(call_id, buffer, buffer_size);
}

int ocall_faasm_get_current_idx(void) {
    return wasm::getExecutingCall()->idx();
}

}