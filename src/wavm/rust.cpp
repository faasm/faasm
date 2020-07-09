#include "WAVMWasmModule.h"
#include "syscalls.h"

#include <cstdio>

using namespace WAVM;

namespace wasm {

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "__println", void, __println, I32 strPtr) {
        const std::string toPrint = getStringFromWasm(strPtr);
        printf("%s\n", toPrint.c_str());
    }

    void rustLink() {

    }
}
