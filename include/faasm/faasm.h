#ifndef _FAASM_H
#define _FAASM_H 1

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define MAX_CHAINS 20
#define MAX_NAME_LENGTH 32
#define MAX_INPUT_BYTES 65536
#define MAX_OUTPUT_BYTES 65536
#define MAX_STATE_BYTES 65536

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

namespace faasm {

    class FaasmMemory {
    public:
        FaasmMemory(uint8_t *in, uint8_t *out, uint8_t *cFuncs, uint8_t *cIn) {
            input = in;
            output = out;

            chainFunctions = cFuncs;
            chainInputs = cIn;
            chainCount = 0;
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

        /**
         * Returns a pointer to the input data for this function
         */
        const uint8_t *getInput() {
            return input;
        };

        /**
         * Sets the given array as the output data for this function
         */
        void setOutput(const uint8_t *newOutput, size_t outputLen) {
            memcpy(this->output, newOutput, outputLen);
        }

        /**
         * Creates a "chained" function call to another function owned by
         * the same user with the given input data
         */
        void chainFunction(const char *name, const uint8_t *inputData, int inputDataSize) {
            // Work out how many chained functions we already have
            int32_t chainIdx = this->chainCount;

            if (chainIdx > MAX_CHAINS) {
                fprintf(stderr, "%s", "Reached max chains");
                return;
            }

            // Get the memory offsets for name and data
            uint8_t *namePtr = this->chainFunctions + (chainIdx * MAX_NAME_LENGTH);
            uint8_t *dataPtr = this->chainInputs + (chainIdx * MAX_INPUT_BYTES);

            // Copy the data into place
            strcpy((char *) namePtr, name);
            memcpy(dataPtr, inputData, (size_t) inputDataSize);

            // Increment the count
            this->chainCount = chainIdx + 1;
        }

    private:
        uint8_t *input;

        uint8_t *output;

        uint8_t *chainFunctions;
        uint8_t *chainInputs;
        int32_t chainCount;
    };

    /**
    * Function for faasm functions to implement
    */
    int exec(FaasmMemory *memory);
}

FAASM_EXPORT
FAASM_INLINE
int run(uint8_t *in, uint8_t *out, uint8_t *cFuncs, uint8_t *cIn) {
    faasm::FaasmMemory memory(in, out, cFuncs, cIn);

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