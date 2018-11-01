#ifndef _FAASM_H
#define _FAASM_H 1

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define MAX_INPUT_BYTES 65536
#define MAX_OUTPUT_BYTES 65536

// Work out if we're in a wasm cross-compile environment
// This can be used to avoid functions being removed by DCE
#if __clang_major__ == 8
#define FAASM_EXPORT __attribute__((used)) __attribute__ ((visibility ("default")))
#define FAASM_INLINE
#else
#define FAASM_EXPORT
#define FAASM_INLINE static inline
#endif

// Intrinsic functions implemented by the runtime
size_t __faasm_read_state(const char *key, uint8_t *buffer, size_t bufferLen);

void __faasm_write_state(const char *key, uint8_t *data, size_t dataLen);

void __faasm_write_state_offset(const char *key, size_t offset, uint8_t *data, size_t dataLen);

void __faasm_read_state_offset(const char *key, size_t offset, uint8_t *buffer, size_t bufferLen);

size_t __faasm_get_input(uint8_t *buffer, size_t bufferLen);

void __faasm_set_output(const uint8_t *output, size_t outputLen);

void __faasm_chain_function(const char *name, const uint8_t *inputData, size_t inputDataSize);


namespace faasm {

    class FaasmMemory {
    public:
        FaasmMemory() {
        };

        /** Returns the size of the state in bytes. Returns zero if not set. */
        size_t getStateSize(const char *key) {
            uint8_t buf[1];

            // Passing zero buffer len returns total size
            return __faasm_read_state(key, buf, 0);
        }

        /**
         * Reads the full state at the given key
         */
        void readState(const char *key, uint8_t *buffer, size_t bufferLen) {
            __faasm_read_state(key, buffer, bufferLen);
        };

        /**
         * Overwrites the state at the given key
         */
        void writeState(const char *key, uint8_t *data, size_t dataLen) {
            __faasm_write_state(key, data, dataLen);
        };

        /**
         * Writes a chunk of state at the given key and offset
         */
        void writeStateOffset(const char *key, size_t offset, uint8_t *data, size_t dataLen) {
            __faasm_write_state_offset(key, offset, data, dataLen);
        };

        /**
         * Reads a chunk of state at the given key and offset
         */
        void readStateOffset(const char *key, size_t offset, uint8_t *buffer, size_t bufferLen) {
            __faasm_read_state_offset(key, offset, buffer, bufferLen);
        };

        /** Returns the size of the input in bytes. Returns zero if none. */
        size_t getInputSize() {
            uint8_t buf[1];

            // Passing zero buffer len returns total size
            return __faasm_get_input(buf, 0);
        }

        /**
         * Returns a pointer to the input data for this function
         */
        const uint8_t *getInput() {
            auto inputArray = new uint8_t[MAX_INPUT_BYTES];
            __faasm_get_input(inputArray, MAX_INPUT_BYTES);

            return inputArray;
        };

        /**
         * Sets the given array as the output data for this function
         */
        void setOutput(const uint8_t *newOutput, size_t outputLen) {
            __faasm_set_output(newOutput, outputLen);
        }

        /**
         * Creates a "chained" function call to another function owned by the same user
         */
        void chainFunction(const char *name) {
            this->chainFunction(name, nullptr, 0);
        }

        /**
         * Chains a function with the given input data
         */
        void chainFunction(const char *name, const uint8_t *inputData, size_t inputDataSize) {
            __faasm_chain_function(name, inputData, inputDataSize);
        }
    };

    /**
    * Function for faasm functions to implement
    */
    int exec(FaasmMemory *memory);
}

FAASM_INLINE
FAASM_EXPORT
int run() {
    faasm::FaasmMemory memory;

    return faasm::exec(&memory);
}

#if __clang_major__ == 8
// Dummy main function to keep compiler happy
int main(int a, char* args[]) {
    uint8_t foo = 42;
    run(&foo, &foo, &foo, &foo);

    return 0;
}
#endif

#endif