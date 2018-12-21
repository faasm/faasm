#include <faasm/memory.h>
#include <util/util.h>
#include <infra/infra.h>

/**
 * Dummy implementations of state calls to keep compiler happy
 */

extern "C" {

long __faasm_read_state(const char *key, unsigned char *buffer, long bufferLen, int async) {
    infra::Redis *redis = infra::Redis::getThreadState();
    std::vector<uint8_t> state = redis->get(key);
    int stateSize = util::safeCopyToBuffer(state, buffer, bufferLen);
    return stateSize;
}

void __faasm_write_state(const char *key, const unsigned char *data, long dataLen, int async) {
    infra::Redis *redis = infra::Redis::getThreadState();

    if (dataLen > 0) {
        std::vector<uint8_t> newState(data, data + dataLen);
        redis->set(key, newState);
    }
}

void __faasm_write_state_offset(const char *key, long offset, const unsigned char *data, long dataLen, int async) {
    infra::Redis *redis = infra::Redis::getThreadState();
    std::vector<uint8_t> newState(data, data + dataLen);
    redis->setRange(key, offset, newState);
}

void __faasm_read_state_offset(const char *key, long offset, unsigned char *buffer, long bufferLen, int async) {
    infra::Redis *redis = infra::Redis::getThreadState();
    std::vector<uint8_t> state = redis->getRange(key, offset, offset + bufferLen);
    util::safeCopyToBuffer(state, buffer, bufferLen);
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

}

