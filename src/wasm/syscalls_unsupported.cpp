#include "WasmModule.h"
#include "syscalls.h"

#include <util/bytes.h>

#include <WAVM/Runtime/Runtime.h>
#include <WAVM/Runtime/RuntimeData.h>
#include <WAVM/Runtime/Intrinsics.h>

namespace wasm {
    // ------------------------
    // Signals (ignored)
    // ------------------------

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

    DEFINE_INTRINSIC_FUNCTION(env, "getpriority", I32, getpriority, I32 a, I32 b) {
        util::getLogger()->debug("S - getpriority - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "setpriority", I32, setpriority, I32 a, I32 b, I32 c) {
        util::getLogger()->debug("S - setpriority - {} {} {}", a, b, c);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "openpty", I32, openpty, I32 a, I32 b, I32 c, I32 d, I32 e) {
        util::getLogger()->debug("S - openpty - {} {} {} {} {}", a, b, c, d, e);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "forkpty", I32, forkpty, I32 a, I32 b, I32 c, I32 d) {
        util::getLogger()->debug("S - forkpty - {} {} {} {}", a, b, c, d);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "lockf", I32, lockf, I32 a, I32 b, I64 c) {
        util::getLogger()->debug("S - lockf - {} {} {}", a, b, c);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "getresuid", I32, getresuid, I32 a, I32 b, I32 c) {
        util::getLogger()->debug("S - getresuid - {} {} {}", a, b, c);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "getresgid", I32, getresgid, I32 a, I32 b, I32 c) {
        util::getLogger()->debug("S - getresgid - {} {} {}", a, b, c);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "getrusage", I32, getrusage, I32 a, I32 b) {
        util::getLogger()->debug("S - getrusage - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "getrlimit", I32, getrlimit, I32 a, I32 b) {
        util::getLogger()->debug("S - getrlimit - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "setrlimit", I32, setrlimit, I32 a, I32 b) {
        util::getLogger()->debug("S - setrlimit - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__copy_tls", I32, __copy_tls, I32 a) {
        util::getLogger()->debug("S - __copy_tls - {}", a);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "realpath", I32, realpath, I32 a, U32 b) {
        util::getLogger()->debug("S - realpath - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "longjmp", void, longjmp, I32 a, U32 b) {
        util::getLogger()->debug("S - longjmp - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "setjmp", I32, setjmp, I32 a) {
        util::getLogger()->debug("S - setjmp - {}", a);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "strtold_l", void, strtold_l, I32 a, I32 b, I32 c, I32 d) {
        util::getLogger()->debug("S - strtold_l - {} {} {} {}", a, b, c, d);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "strpbrk", I32, strpbrk, I32 a, I32 b) {
        util::getLogger()->debug("S - strpbrk - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }
}