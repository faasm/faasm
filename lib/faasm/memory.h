#ifndef FAASM_MEMORY_H
#define FAASM_MEMORY_H

#include <stdint.h>

namespace faasm {

    class FaasmMemory {
    public:
        FaasmMemory();

        /**
         * Returns the size of the state in bytes. Returns zero if not set.
         */
        long getStateSize(const char *key);

        /**
         * Reads the full state at the given key
         */
        long readState(const char *key, uint8_t *buffer, long bufferLen);

        /**
         * Overwrites the state at the given key
         */
        void writeState(const char *key, uint8_t *data, long dataLen);

        /**
         * Writes a chunk of state at the given key and offset
         */
        void writeStateOffset(const char *key, long offset, uint8_t *data, long dataLen);

        /**
         * Reads a chunk of state at the given key and offset
         */
        void readStateOffset(const char *key, long offset, uint8_t *buffer, long bufferLen);

        /**
         * Returns the size of the input in bytes. Returns zero if none.
         * */
        long getInputSize();

        /**
         * Returns a pointer to the input data for this function
         */
        const uint8_t *getInput();

        /**
         * Sets the given array as the output data for this function
         */
        void setOutput(const uint8_t *newOutput, long outputLen);

        /**
         * Creates a "chained" function call to another function owned by the same user
         */
        void chainFunction(const char *name);

        /**
         * Chains a function with the given input data
         */
        void chainFunction(const char *name, const uint8_t *inputData, long inputDataSize);
    };
}

#endif
