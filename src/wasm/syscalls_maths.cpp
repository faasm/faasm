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

//    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "dgelsd_", I32, dgelsd_, I32 a, I32 b, I32 c, I32 d, I32 e, I32 f, I32 g, I32 h,
//                                   I32 i, I32 j, I32 k, I32 l, I32 m, I32 n) {
//        util::getLogger()->debug("S - dgelsd_");
//
//        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
//    }
//
//    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "dgeqrf_", I32, dgeqrf_, I32 a, I32 b, I32 c, I32 d, I32 e, I32 f, I32 g,
//                                   I32 h) {
//        util::getLogger()->debug("S - dgeqrf_");
//
//        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
//    }
//
//    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "dorgqr_", I32, dorgqr_, I32 a, I32 b, I32 c, I32 d, I32 e, I32 f, I32 g, I32 h,
//                                   I32 i) {
//        util::getLogger()->debug("S - dorgqr_");
//
//        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
//    }
//
//    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "zgelsd_", I32, zgelsd_, I32 a, I32 b, I32 c, I32 d, I32 e, I32 f, I32 g, I32 h,
//                                   I32 i, I32 j, I32 k, I32 l, I32 m, I32 n, I32 o) {
//        util::getLogger()->debug("S - zgelsd_");
//
//        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
//    }
//
//    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "zgeqrf_", I32, zgeqrf_, I32 a, I32 b, I32 c, I32 d, I32 e, I32 f, I32 g,
//                                   I32 h) {
//        util::getLogger()->debug("S - zgeqrf_");
//
//        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
//    }
//
//    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "zungqr_", I32, zungqr_, I32 a, I32 b, I32 c, I32 d, I32 e, I32 f, I32 g, I32 h,
//                                   I32 i) {
//        util::getLogger()->debug("S - zungqr_");
//
//        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
//    }
}