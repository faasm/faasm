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

    // BLAS stuff we've not yet implemented

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "cblas_cdotu_sub", void, cblas_cdotu_sub, I32 N, I32 X, I32 incX, I32 Y,
                                   I32 incY, I32 dotu) {
        util::getLogger()->debug("S - cblas_cdotu_sub");

        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "cblas_cdotc_sub", void, cblas_cdotc_sub, I32 N, I32 X, I32 incX, I32 Y,
                                   I32 incY, I32 dotu) {
        util::getLogger()->debug("S - cblas_cdotc_sub");

        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "cblas_zdotu_sub", void, cblas_zdotu_sub, I32 N, I32 X, I32 incX, I32 Y,
                                   I32 incY, I32 dotu) {
        util::getLogger()->debug("S - cblas_zdotu_sub");

        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "cblas_zdotc_sub", void, cblas_zdotc_sub, I32 N, I32 X, I32 incX, I32 Y,
                                   I32 incY, I32 dotu) {
        util::getLogger()->debug("S - cblas_zdotc_sub");

        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }
}