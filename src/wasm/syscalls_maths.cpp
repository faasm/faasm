#include "WasmModule.h"
#include "syscalls.h"
#include <math.h>

#include <WAVM/Runtime/Runtime.h>
#include <WAVM/Runtime/Intrinsics.h>

namespace wasm {
    void mathsLink() {

    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "exp2", F64, math_exp2, F64 a) {
        util::getLogger()->debug("S - exp2 - {}", a);
        return exp2(a);
    }
}