#include "WasmModule.h"

#include <cstdio>

namespace wasm {

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "__println", void, __println, I32 _buffer) {
        Runtime::Memory *memoryPtr = getExecutingModule()->defaultMemory;
        const char *buffer = (const char *) &Runtime::memoryRef<I32>(memoryPtr, _buffer);
        printf("%s\n", buffer);
    }

    void rustLink() {

    }
}
