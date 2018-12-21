#include "faasm/memory.h"

extern "C" {
long __faasm_read_state(const char *key, unsigned char *buffer, long bufferLen, int async);

void __faasm_write_state(const char *key, const unsigned char *data, long dataLen, int async);

void __faasm_write_state_offset(const char *key, long offset, const unsigned char *data, long dataLen, int async);

void __faasm_read_state_offset(const char *key, long offset, unsigned char *buffer, long bufferLen, int async);

void __faasm_lock_state_read(const char *key);

void __faasm_unlock_state_read(const char *key);

void __faasm_lock_state_write(const char *key);

void __faasm_unlock_state_write(const char *key);

long __faasm_read_input(unsigned char *buffer, long bufferLen);

void __faasm_write_output(const unsigned char *output, long outputLen);

void __faasm_chain_function(const char *name, const unsigned char *inputData, long inputDataSize);
}

namespace faasm {

    FaasmMemory::FaasmMemory() {

    }

    long FaasmMemory::getStateSize(const char *key, bool async) {
        // Passing zero buffer len returns total size
        uint8_t buf[1];
        return __faasm_read_state(key, buf, 0, (int) async);
    }

    long FaasmMemory::readState(const char *key, uint8_t *buffer, long bufferLen, bool async) {
        return __faasm_read_state(key, buffer, bufferLen, (int) async);
    };

    void FaasmMemory::writeState(const char *key, const uint8_t *data, long dataLen, bool async) {
        __faasm_write_state(key, data, dataLen, (int) async);
    };

    void FaasmMemory::writeStateOffset(const char *key, long offset, const uint8_t *data, long dataLen, bool async) {
        __faasm_write_state_offset(key, offset, data, dataLen, (int) async);
    };

    void FaasmMemory::readStateOffset(const char *key, long offset, uint8_t *buffer, long bufferLen, bool async) {
        __faasm_read_state_offset(key, offset, buffer, bufferLen, (int) async);
    };

    long FaasmMemory::getInputSize() {
        uint8_t buf[1];

        // Passing zero buffer len returns total size
        return __faasm_read_input(buf, 0);
    }

    void FaasmMemory::getInput(uint8_t *buffer, long bufferLen) {
        __faasm_read_input(buffer, bufferLen);
    };

    void FaasmMemory::setOutput(const uint8_t *newOutput, long outputLen) {
        __faasm_write_output(newOutput, outputLen);
    }

    void FaasmMemory::chainFunction(const char *name) {
        this->chainFunction(name, nullptr, 0);
    }

    void FaasmMemory::chainFunction(const char *name, const uint8_t *inputData, long inputDataSize) {
        __faasm_chain_function(name, inputData, inputDataSize);
    }
};
