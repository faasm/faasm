#include <faasm/faasm.h>
#include <util/util.h>
#include <infra/infra.h>

/**
 * Dummy implementations of state calls to keep compiler happy
 */

static thread_local infra::Redis redis;

size_t __faasm_read_state(const char *key, uint8_t *buffer, size_t bufferLen) {
    std::vector<uint8_t> state = redis.get(key);
    int stateSize = util::safeCopyToBuffer(state, buffer, bufferLen);
    return stateSize;
}

void __faasm_write_state(const char *key, uint8_t *data, size_t dataLen) {
    std::vector<uint8_t> newState(data, data + dataLen);
    redis.set(key, newState);
}

void __faasm_write_state_offset(const char *key, size_t offset, uint8_t *data, size_t dataLen) {
    std::vector<uint8_t> newState(data, data + dataLen);
    redis.setRange(key, offset, newState);
}

void __faasm_read_state_offset(const char *key, size_t offset, uint8_t *buffer, size_t bufferLen) {
    std::vector<uint8_t> state = redis.getRange(key, offset, offset + bufferLen);
    util::safeCopyToBuffer(state, buffer, bufferLen);
}

size_t __faasm_get_input(uint8_t *buffer, size_t bufferLen) {
    std::vector<uint8_t> inputData = {1, 2, 3, 4, 5};
    std::copy(inputData.begin(), inputData.end(), buffer);

    return 14;
}

void __faasm_set_output(const uint8_t *output, size_t outputLen) {

}

void __faasm_chain_function(const char *name, const uint8_t *inputData, size_t inputDataSize) {

}
