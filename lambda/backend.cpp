#include "backend.h"

#include "faasm/memory.h"

#include <string>

namespace faasm {

    std::string input;
    std::string output;

    void startRequest() {
        printf("Starting request\n");

        input = std::string();
        output = std::string();
    }

    void setInput(const std::string &i) {
        printf("Received input [%s]\n", i.c_str());
        input = i;
    }

    std::string getOutput() {
        printf("Returning output [%s]\n", output.c_str());
        return output;
    }

    void initialiseLambdaBackend() {

    }
}

long __faasm_read_input(unsigned char *buffer, long bufferLen) {
    if(bufferLen == 0) {
        return faasm::input.size();
    }

    auto inputBytes = reinterpret_cast<uint8_t *>(faasm::input.data());
    std::copy(inputBytes, inputBytes + faasm::input.size(), buffer);

    return bufferLen;
}

void __faasm_write_output(const unsigned char *o, long len) {
    auto charPtr = reinterpret_cast<const char*>(o);
    faasm::output = std::string(charPtr, (size_t) len);
}

void __faasm_chain_function(const char *name, const unsigned char *inputData, long inputDataSize) {
    // TODO use lambda api to call another function?
    // Requires AWS SDK?
}

void __faasm_read_state(const char *key, unsigned char *buffer, long bufferLen, int async) {
    // TODO read from Redis
}

unsigned char *__faasm_read_state_ptr(const char *key, long totalLen, int async) {
    // TODO read from Redis
    auto ptr = new unsigned char[1];
    return ptr;
}

void __faasm_read_state_offset(const char *key, long totalLen, long offset, unsigned char *buffer, long bufferLen,
                               int async) {
    // TODO read from Redis
}

unsigned char *__faasm_read_state_offset_ptr(const char *key, long totalLen, long offset, long len, int async) {
    // TODO read from Redis
    auto ptr = new unsigned char[1];
    return ptr;
}

void __faasm_write_state(const char *key, const uint8_t *data, long dataLen, int async) {
    // TODO write to Redis
}

void __faasm_write_state_offset(const char *key, long totalLen, long offset, const unsigned char *data, long dataLen,
                                int async) {
    // TODO write to Redis
}

// Ignore everything from here?

void __faasm_flag_state_dirty(const char *key, long totalLen) {

}

void __faasm_flag_state_offset_dirty(const char *key, long totalLen, long offset, long dataLen) {

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

