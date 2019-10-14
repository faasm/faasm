#include "knative_emulator.h"

extern "C" {
#include <faasm/core.h>
#include <faasm/host_interface.h>
}

static std::vector<uint8_t> outputData;
static std::vector<uint8_t> inputData;

namespace knative_native {
    std::vector<uint8_t> getEmulatorOutputData() {
        return outputData;
    }

    void setEmulatorInputData(const std::vector<uint8_t> &inputIn) {
        inputData = inputIn;
    }
}

long __faasm_read_input(unsigned char *buffer, long bufferLen) {
    long inputLen = inputData.size();

    if (bufferLen == 0) {
        return inputLen;
    }

    if (inputLen == 0) {
        return 0;
    }

    std::copy(inputData.data(), inputData.data() + inputLen, buffer);
    return bufferLen;
}

void __faasm_write_output(const unsigned char *output, long outputLen) {
    outputData = std::vector<uint8_t>(output, output + outputLen);
}


void __faasm_read_state(const char *key, unsigned char *buffer, long bufferLen, int async) {
    // TODO
}

unsigned char *__faasm_read_state_ptr(const char *key, long totalLen, int async) {
    // TODO
    return nullptr;
}


void __faasm_read_state_offset(const char *key, long totalLen, long offset, unsigned char *buffer, long bufferLen,
                               int async) {
    // TODO
}

unsigned char *__faasm_read_state_offset_ptr(const char *key, long totalLen, long offset, long len, int async) {
    // TODO
    return nullptr;
}

void __faasm_write_state(const char *key, const uint8_t *data, long dataLen, int async) {
    // TODO
}

void __faasm_write_state_offset(const char *key, long totalLen, long offset, const unsigned char *data, long dataLen,
                                int async) {
    // TODO
}

void __faasm_flag_state_dirty(const char *key, long totalLen) {
    // Ignore
}

void __faasm_flag_state_offset_dirty(const char *key, long totalLen, long offset, long dataLen) {
    // Ignore
}

void __faasm_push_state(const char *key) {
    // Ignore
}

void __faasm_push_state_partial(const char *key) {
    // Ignore
}


unsigned int __faasm_chain_this(int idx, const unsigned char *buffer, long bufferLen) {
    // TODO - invoke with knative interface
    return 0;
}

int __faasm_await_call(unsigned int callId) {
    // TODO - handle with knative
    return 0;
}

int __faasm_get_idx() {
    // TODO - is this needed?
    return 0;
}

unsigned int __faasm_chain_function(const char *name, const unsigned char *inputData, long inputDataSize) {
    // TODO - is this needed?
    return 1;
}

void __faasm_read_config(const char *varName, char *buffer) {
    // TODO - Is this needed?
}

void __faasm_lock_state_read(const char *key) {

}

void __faasm_unlock_state_read(const char *key) {

}

void __faasm_lock_state_write(const char *key) {

}

void __faasm_unlock_state_write(const char *key) {

}
