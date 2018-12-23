#include <faasm/memory.h>
#include <util/util.h>
#include <infra/infra.h>
#include <state/state.h>

/**
 * Local implementations of faasm calls
 */

state::StateKeyValue *getKv(const char *key, size_t size) {
    state::State &s = state::getGlobalState();
    state::StateKeyValue *kv = s.getKV("demo", key, size);
    return kv;
}

void __faasm_read_state(const char *key, unsigned char *buffer, long bufferLen, int async) {
    state::StateKeyValue *kv = getKv(key, bufferLen);

    bool isAsync = async == 1;
    kv->pull(isAsync);

    kv->get(buffer);
}

void __faasm_read_state_offset(const char *key, long totalLen, long offset, unsigned char *buffer, long bufferLen,
                               int async) {
    state::StateKeyValue *kv = getKv(key, totalLen);

    bool isAsync = async == 1;
    kv->pull(isAsync);

    kv->getSegment(offset, buffer, bufferLen);
}

void __faasm_write_state(const char *key, const uint8_t *data, long dataLen, int async) {
    state::StateKeyValue *kv = getKv(key, dataLen);
    kv->set(data);

    if (async == 0) {
        kv->pushFull();
    }
}

void __faasm_write_state_offset(const char *key, long totalLen, long offset, const unsigned char *data, long dataLen,
                                int async) {
    state::StateKeyValue *kv = getKv(key, totalLen);

    kv->setSegment(offset, data, dataLen);

    if (async == 0) {
        kv->pushPartial();
    }
}

void __faasm_push_state(const char *key) {
    state::StateKeyValue *kv = getKv(key, 0);
    kv->pushFull();
}

void __faasm_push_state_partial(const char *key) {
    state::StateKeyValue *kv = getKv(key, 0);
    kv->pushPartial();
}

long __faasm_read_input(unsigned char *buffer, long bufferLen) {
    std::vector<uint8_t> inputData = {1, 2, 3, 4, 5};
    std::copy(inputData.begin(), inputData.end(), buffer);

    return 14;
}

void __faasm_lock_state_read(const char *key) {

}

void __faasm_unlock_state_read(const char *key) {

}

void __faasm_lock_state_write(const char *key) {

}

void __faasm_unlock_state_write(const char *key) {

}

void __faasm_write_output(const unsigned char *output, long outputLen) {

}

void __faasm_chain_function(const char *name, const unsigned char *inputData, long inputDataSize) {

}


