#include "WasmModule.h"
#include "syscalls.h"

#include <util/bytes.h>

#include <WAVM/Runtime/Runtime.h>
#include <WAVM/Runtime/RuntimeData.h>
#include <WAVM/Runtime/Intrinsics.h>

namespace wasm {

    I32 s__futex(I32 uaddrPtr, I32 futex_op, I32 val, I32 timeoutPtr, I32 uaddr2Ptr, I32 other) {
        util::getLogger()->debug("S - futex - {} {} {} {} {} {}", uaddrPtr, futex_op, val, timeoutPtr, uaddr2Ptr, other);
        return 0;
    }

    // ------------------------
    // Signals (ignored)
    // ------------------------

    I32 s__sigaltstack(I32 ssPtr, I32 oldSsPtr) {
        util::getLogger()->debug("S - sigaltstack - {} {}", ssPtr, oldSsPtr);
        return 0;
    }

    I32 s__sigaction(I32 a, I32 b, I32 c) {
        util::getLogger()->debug("S - sigaction - {} {} {}", a, b, c);

        return 0;
    }

    I32 s__sigemptyset(I32 a) {
        util::getLogger()->debug("S - sigemptyset - {}", a);

        return 0;
    }

    I32 s__signal(I32 a, I32 b) {
        util::getLogger()->debug("S - signal - {} {}", a, b);

        return 0;
    }

    I32 s__siginterrupt(I32 a, I32 b) {
        util::getLogger()->debug("S - siginterrupt - {} {}", a, b);

        return 0;
    }

    I32 s__rt_sigprocmask(I32 how, I32 sigSetPtr, I32 oldSetPtr, I32 sigsetsize) {
        util::getLogger()->debug("S - rt_sigprocmask - {} {} {} {}", how, sigSetPtr, oldSetPtr, sigsetsize);

        return 0;
    }

    // --------------------------
    // Exceptions
    // --------------------------

    DEFINE_INTRINSIC_FUNCTION(env, "_Unwind_RaiseException", I32, _Unwind_RaiseException, I32 a) {
        util::getLogger()->debug("S - _Unwind_RaiseException - {}", a);
        return 0;
    }

    DEFINE_INTRINSIC_FUNCTION(env, "_Unwind_DeleteException", void, _Unwind_DeleteException, I32 a) {
        util::getLogger()->debug("S - _Unwind_DeleteException - {}", a);
    }

    // --------------------------
    // Python-related dumping ground
    // --------------------------

    DEFINE_INTRINSIC_FUNCTION(env, "zlibVersion", I32, emscripten_zlibVersion ) {
        util::getLogger()->debug("S - zlibVersion");
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "deflateEnd", I32, emscripten_deflateEnd , I32 a) {
        util::getLogger()->debug("S - deflateEnd - {}", a);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "deflate", I32, emscripten_deflate , I32 a, I32 b) {
        util::getLogger()->debug("S - deflate - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "inflateEnd", I32, emscripten_inflateEnd , I32 a) {
        util::getLogger()->debug("S - inflateEnd - {}", a);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "inflate", I32, emscripten_inflate , I32 a, I32 b) {
        util::getLogger()->debug("S - inflate - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "inflateSetDictionary", I32, emscripten_inflateSetDictionary , I32 a, I32 b, I32 c) {
        util::getLogger()->debug("S - inflateSetDictionary - {} {} {}", a, b, c);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "adler32", I32, emscripten_adler32 , I32 a, I32 b, I32 c) {
        util::getLogger()->debug("S - adler32 - {} {} {}", a, b, c);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "deflateInit_", I32, emscripten_deflateInit_ , I32 a, I32 b, I32 c, I32 d) {
        util::getLogger()->debug("S - deflateInit_ - {} {} {} {}", a, b, c, d);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "deflateInit2_", I32, emscripten_deflateInit2_ , I32 a, I32 b, I32 c, I32 d, I32 e, I32 f, I32 g, I32 h) {
        util::getLogger()->debug("S - deflateInit2_ - {} {} {} {} {} {} {} {}", a, b, c, d, e, f, g, h);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "deflateSetDictionary", I32, emscripten_deflateSetDictionary , I32 a, I32 b, I32 c) {
        util::getLogger()->debug("S - deflateSetDictionary - {} {} {}", a, b, c);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "crc32", I32, emscripten_crc32 , I32 a, I32 b, I32 c) {
        util::getLogger()->debug("S - crc32 - {} {} {}", a, b, c);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "inflateInit2_", I32, emscripten_inflateInit2_ , I32 a, I32 b, I32 c, I32 d) {
        util::getLogger()->debug("S - inflateInit2_ - {} {} {} {}", a, b, c, d);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "getpriority", I32, emscripten_getpriority , I32 a, I32 b) {
        util::getLogger()->debug("S - getpriority - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "setpriority", I32, emscripten_setpriority , I32 a, I32 b, I32 c) {
        util::getLogger()->debug("S - setpriority - {} {} {}", a, b, c);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "uname", I32, emscripten_uname , I32 a) {
        util::getLogger()->debug("S - uname - {}", a);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "openpty", I32, emscripten_openpty , I32 a, I32 b, I32 c, I32 d, I32 e) {
        util::getLogger()->debug("S - openpty - {} {} {} {} {}", a, b, c, d, e);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "forkpty", I32, emscripten_forkpty , I32 a, I32 b, I32 c, I32 d) {
        util::getLogger()->debug("S - forkpty - {} {} {} {}", a, b, c, d);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "lockf", I32, emscripten_lockf , I32 a, I32 b, I64 c) {
        util::getLogger()->debug("S - lockf - {} {} {}", a, b, c);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "getresuid", I32, emscripten_getresuid , I32 a, I32 b, I32 c) {
        util::getLogger()->debug("S - getresuid - {} {} {}", a, b, c);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "getresgid", I32, emscripten_getresgid , I32 a, I32 b, I32 c) {
        util::getLogger()->debug("S - getresgid - {} {} {}", a, b, c);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "getrusage", I32, emscripten_getrusage , I32 a, I32 b) {
        util::getLogger()->debug("S - getrusage - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "getrlimit", I32, emscripten_getrlimit , I32 a, I32 b) {
        util::getLogger()->debug("S - getrlimit - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "setrlimit", I32, emscripten_setrlimit , I32 a, I32 b) {
        util::getLogger()->debug("S - setrlimit - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__copy_tls", I32, emscripten___copy_tls , I32 a) {
        util::getLogger()->debug("S - __copy_tls - {}", a);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "realpath", I32, realpath, I32 a, U32 b) {
        util::getLogger()->debug("S - realpath - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }
}