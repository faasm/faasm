#include "WAVMWasmModule.h"
#include "syscalls.h"
#include <math.h>

#include <WAVM/Runtime/Runtime.h>
#include <WAVM/Runtime/Intrinsics.h>

using namespace WAVM;

namespace wasm {
    void mathsLink() {

    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "exp2", F64, math_exp2, F64 a) {
        return exp2(a);
    }
    
    // -----------------------------------
    // FLOATS
    // -----------------------------------

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "feclearexcept", I32, wasi_feclearexcept, I32 a) {
        faabric::util::getLogger()->debug("S - feclearexcept - {}", a);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }
    
    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "feraiseexcept", I32, wasi_feraiseexcept, I32 a) {
        faabric::util::getLogger()->debug("S - feraiseexcept - {}", a);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }
    
    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "fetestexcept", I32, wasi_fetestexcept, I32 a) {
        faabric::util::getLogger()->debug("S - fetestexcept - {}", a);

        // Here we always say there a no float-related exceptions

        return 0;
    }

    // ------------------------------------
    // UNSUPPORTED
    // ------------------------------------

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "Dragon4_PrintFloat_IEEE_binary128_le", I32, Dragon4_PrintFloat_IEEE_binary128_le, I32 a, I32 b, I32 c) {
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "fma", F64, fma, F64 a, F64 b, F64 c) {
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }
    
    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "__fixunstfsi", I32, wasi__fixunstfsi, I64 a, I64 b) {
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }
    
    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "__fixunstfdi", I64, wasi__fixunstfdi, I64 a, I64 b) {
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "__fixtfsi", I32, wasi__fixtfsi, I64 a, I64 b) {
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "__netf2", I32, wasi__netf2, I64 a, I64 b, I64 c, I64 d) {
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "__floatunditf", void, wasi__floatunditf, I32 a, I64 b) {
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }
    
    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "__floatditf", void, wasi__floatditf, I32 a, I64 b) {
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }
    
    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "__floatsitf", void, wasi__floatsitf, I32 a, I32 b) {
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }
    
    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "__floatunsitf", void, wasi__floatunsitf, I32 a, I32 b) {
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "__trunctfsf2", F32, wasi__trunctfsf2, I64 a, I64 b) {
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "__fixtfdi", I64, wasi__fixtfdi, I64 a, I64 b) {
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
