#ifndef FAASM_MEMORY_H
#define FAASM_MEMORY_H

#include <stdint.h>

namespace faasm {

    class FaasmMemory {
    public:
        FaasmMemory();

        /**
         * Reads the full state at the given key
         */
        void readState(const char *key, uint8_t *buffer, long bufferLen, bool async = false);

        /**
         * Reads the full state and returns a direct pointer
         */
        uint8_t *readState(const char *key, long totalLen, bool async);

        /**
        * Reads a chunk of state at the given key and offset
        */
        void readStateOffset(const char *key, long totalLen, long offset, uint8_t *buffer, long bufferLen,
                             bool async = false);

        /**
          * Reads a chunk of state and returns a direct pointer
          */
        uint8_t *readStateOffset(const char *key, long fullLen, long offset, long len, bool async);

        /**
         * Overwrites the state at the given key
         */
        void writeState(const char *key, const uint8_t *data, long dataLen, bool async = false);

        /**
         * Writes a chunk of state at the given key and offset
         */
        void writeStateOffset(const char *key, long totalLen, long offset, const uint8_t *data, long dataLen,
                              bool async = false);

        /**
         * Mark the whole value as dirty
         */
        void flagStateDirty(const char *key, long totalLen);

        /**
         * Mark a segment as dirty
         */
        void flagStateOffsetDirty(const char *key, long totalLen, long offset, long len);

        /**
         * Forces a push of state
         */
        void pushState(const char *key);

        /**
         * Forces a push of any partial state updates
         */
        void pushStatePartial(const char *key);

        /**
        * Acquires a read lock for the given state
        */
        void lockStateRead(const char *key);

        /**
        * Releases a read lock for the given state
        */
        void unlockStateRead(const char *key);

        /**
         * Acquires a write lock for the given state
         */
        void lockStateWrite(const char *key);

        /**
        * Releases a write lock for the given state
        */
        void unlockStateWrite(const char *key);

        /**
         * Returns the size of the input in bytes. Returns zero if none.
         * */
        long getInputSize();

        /**
         * Returns a pointer to the input data for this function
         */
        void getInput(uint8_t *buffer, long bufferLen);

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
