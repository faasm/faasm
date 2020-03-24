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
        AsyncArray(const char *keyIn, size_t nElemsIn) :
                key(keyIn),
                nElems(nElemsIn),
                bytesSize(nElems * sizeof(T)),
                contents(nullptr) {
        }

        void zero() {
            faasm::zeroState(key, bytesSize);
            pullLazy();
        }

        T &operator[](int idx) {
            // Assume any accessed data is dirty
            size_t offset = idx * sizeof(T);
            faasmFlagStateOffsetDirty(key, bytesSize, offset, sizeof(T));

            return contents[idx];
        }

        void pullLazy() {
            contents = reinterpret_cast<T *>(faasmReadStatePtr(key, bytesSize));
        }

        void pull() {
            faasmPullState(key, bytesSize);
            pullLazy();
        }

        void push() {
            faasmPushStatePartial(key);
        }

        T *data() {
            return contents;
        }

    private:
        const char *key;
        size_t nElems;
        size_t bytesSize;
        T *contents;
    };

}

#endif
