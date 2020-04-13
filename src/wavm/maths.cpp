#include "WAVMWasmModule.h"
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

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "__trunctfdf2", F64, wasi__trunctfdf2, I64 a, I64 b) {
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "__unordtf2", I32, wasi__unordtf2, I64 a, I64 b, I64 c, I64 d) {
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "__eqtf2", I32, wasi__eqtf2, I64 a, I64 b, I64 c, I64 d) {
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "__signbitl", I32, wasi__signbitl, I64 a, I64 b) {
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "__extendsftf2", void, wasi__extendsftf2, I32 a, F32 b) {
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "__lttf2", I32, wasi__lttf2, I64 a, I64 b, I64 c, I64 d) {
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "__getf2", I32, wasi__getf2, I64 a, I64 b, I64 c, I64 d) {
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "__extenddftf2", void, wasi__extenddftf2, I32 a, F64 b) {
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "__multf3", void, wasi__multf3, I32 a, I64 b, I64 c, I64 d, I64 e) {
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "__addtf3", void, wasi__addtf3, I32 a, I64 b, I64 c, I64 d, I64 e) {
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "__letf2", I32, wasi__letf2, I64 a, I64 b, I64 c, I64 d) {
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "__gttf2", I32, wasi__gttf2, I64 a, I64 b, I64 c, I64 d) {
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "__divtf3", void, wasi__divtf3, I32 a, I64 b, I64 c, I64 d, I64 e) {
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "__subtf3", void, wasi__subtf3, I32 a, I64 b, I64 c, I64 d, I64 e) {
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }
}