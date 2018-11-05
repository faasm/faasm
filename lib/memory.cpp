#include "faasm/memory.h"

extern "C" {
long __faasm_read_state(const char *key, unsigned char *buffer, long bufferLen);

void __faasm_write_state(const char *key, unsigned char *data, long dataLen);

void __faasm_write_state_offset(const char *key, long offset, unsigned char *data, long dataLen);

void __faasm_read_state_offset(const char *key, long offset, unsigned char *buffer, long bufferLen);

long __faasm_read_input(unsigned char *buffer, long bufferLen);

void __faasm_write_output(const unsigned char *output, long outputLen);

void __faasm_chain_function(const char *name, const unsigned char *inputData, long inputDataSize);
}

namespace faasm {

    static const int MAX_INPUT_BYTES = 65536;

    FaasmMemory::FaasmMemory() {

    }

    long FaasmMemory::getStateSize(const char *key) {
        uint8_t buf[1];

        // Passing zero buffer len returns total size
        return __faasm_read_state(key, buf, 0);
    }

    long FaasmMemory::readState(const char *key, uint8_t *buffer, long bufferLen) {
        return __faasm_read_state(key, buffer, bufferLen);
    };

    void FaasmMemory::writeState(const char *key, uint8_t *data, long dataLen) {
        __faasm_write_state(key, data, dataLen);
    };

    void FaasmMemory::writeStateOffset(const char *key, long offset, uint8_t *data, long dataLen) {
        __faasm_write_state_offset(key, offset, data, dataLen);
    };

    void FaasmMemory::readStateOffset(const char *key, long offset, uint8_t *buffer, long bufferLen) {
        __faasm_read_state_offset(key, offset, buffer, bufferLen);
    };

    long FaasmMemory::getInputSize() {
        uint8_t buf[1];

        // Passing zero buffer len returns total size
        return __faasm_read_input(buf, 0);
    }

    const uint8_t *FaasmMemory::getInput() {
        auto inputArray = new uint8_t[MAX_INPUT_BYTES];
        __faasm_read_input(inputArray, MAX_INPUT_BYTES);

        return inputArray;
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
