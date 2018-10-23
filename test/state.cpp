#include <faasm/faasm.h>
#include <infra/infra.h>

/**
 * Dummy implementations of state calls to keep compiler happy
 */

static thread_local infra::Redis redis;

size_t __faasm_read_state(const char *key, uint8_t *buffer, size_t bufferLen) {
    std::vector<uint8_t> state = redis.get(key);
    std::copy(state.data(), state.data() + bufferLen, buffer);
    return state.size();
}

void __faasm_write_state(const char *key, uint8_t *data, size_t dataLen) {
    std::vector<uint8_t> newState(data, data + dataLen);
    redis.set(key, newState);
}

void __faasm_write_state_offset(const char *key, size_t offset, uint8_t *data, size_t dataLen) {
    std::vector<uint8_t> newState(data, data + dataLen);
    redis.setRange(key, offset, newState);
}

