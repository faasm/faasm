#ifndef FAASM_ARRAY_H
#define FAASM_ARRAY_H

#include "faasm/array.h"
#include "faasm/core.h"
#include "faasm/state.h"

#include <cstddef>

namespace faasm {
    template<typename T>
    class AsyncArray {
    public:
        AsyncArray(const char *keyIn, size_t sizeIn) : key(keyIn), size(sizeIn) {
            contents = new T[size];
        }

        ~AsyncArray() {
            delete[] contents;
        }

        void init() {
            faasm::zeroState(key, size * sizeof(T));
        }

        void set(int idx, T val) {
            size_t offset = idx * sizeof(T);
            faasmWriteStateOffset(key, size * sizeof(T), offset, BYTES(&val), sizeof(T));

            contents[idx] = val;
        }

        T get(int idx) {
            return contents[idx];
        }

        void pull() {
            faasmReadState(key, BYTES(contents), size * sizeof(T));
        }

        void push() {
            faasmPushState(key);
        }

        T *data() {
            return contents;
        }

    private:
        const char *key;
        size_t size;
        T *contents;
    };

}

#endif
