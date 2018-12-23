#include <faasm/memory.h>
#include <util/util.h>
#include <infra/infra.h>

/**
 * Dummy implementations of state calls to keep compiler happy
 */

extern "C" {

void __faasm_read_state(const char *key, unsigned char *buffer, long bufferLen, int async) {
    infra::Redis *redis = infra::Redis::getThreadState();
    redis->get(key, buffer, bufferLen);
}

uint8_t *__faasm_read_state_ptr(const char *key, long len, int async) {
    throw std::runtime_error("Not implemented");
}

void __faasm_write_state(const char *key, const unsigned char *data, long dataLen, int async) {
    infra::Redis *redis = infra::Redis::getThreadState();

    if (dataLen > 0) {
        redis->set(key, data, (size_t) dataLen);
    }
}

void __faasm_write_state_offset(const char *key, long totalLen, long offset, const unsigned char *data, long dataLen,
                                int async) {
    infra::Redis *redis = infra::Redis::getThreadState();
    redis->setRange(key, offset, data, dataLen);
}

void __faasm_read_state_offset(const char *key, long totalLen, long offset, unsigned char *buffer, long bufferLen,
                               int async) {
    infra::Redis *redis = infra::Redis::getThreadState();

    // Importantly getRange is *inclusive*, so we need to knock one off the end of the range
    redis->getRange(key, buffer, bufferLen, offset, offset + bufferLen - 1);
}

long __faasm_read_input(unsigned char *buffer, long bufferLen) {
    std::vector<uint8_t> inputData = {1, 2, 3, 4, 5};
    std::copy(inputData.begin(), inputData.end(), buffer);

    return 14;
}

void __faasm_push_state(const char *key) {

}

void __faasm_push_state_partial(const char *key) {

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

