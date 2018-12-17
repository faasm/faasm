#include "faasm/memory.h"

extern "C" {
long __faasm_read_state(const char *key, unsigned char *buffer, long bufferLen);

void __faasm_write_state(const char *key, const unsigned char *data, long dataLen);

void __faasm_write_state_offset(const char *key, long offset, const unsigned char *data, long dataLen);

void __faasm_read_state_offset(const char *key, long offset, unsigned char *buffer, long bufferLen);

long __faasm_read_input(unsigned char *buffer, long bufferLen);

void __faasm_write_output(const unsigned char *output, long outputLen);

void __faasm_chain_function(const char *name, const unsigned char *inputData, long inputDataSize);
}

namespace faasm {

    FaasmMemory::FaasmMemory() {

    }

    long FaasmMemory::getStateSize(const char *key) {
        // Passing zero buffer len returns total size
        uint8_t buf[1];
        return __faasm_read_state(key, buf, 0);
    }

    long FaasmMemory::readState(const char *key, uint8_t *buffer, long bufferLen) {
        return __faasm_read_state(key, buffer, bufferLen);
    };

    void FaasmMemory::writeState(const char *key, const uint8_t *data, long dataLen) {
        __faasm_write_state(key, data, dataLen);
    };

    void FaasmMemory::writeStateOffset(const char *key, long offset, const uint8_t *data, long dataLen) {
        __faasm_write_state_offset(key, offset, data, dataLen);
    };

    void FaasmMemory::forcePushState(const char *key) {
        // TODO - add proper support for this
        // Passing zero is for a force push
        uint8_t buf[1];
        __faasm_write_state(key, buf, 0);
    };

    void FaasmMemory::readStateOffset(const char *key, long offset, uint8_t *buffer, long bufferLen) {
        __faasm_read_state_offset(key, offset, buffer, bufferLen);
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
