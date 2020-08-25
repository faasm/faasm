#include "WAVMWasmModule.h"
#include "syscalls.h"

#include <faabric/util/logging.h>

#include <stdio.h>

#include <WAVM/Runtime/Runtime.h>
#include <WAVM/Runtime/Intrinsics.h>
#include <WAVM/Inline/FloatComponents.h>

using namespace WAVM;

/**
 * SYSCALL NUMBERING
 *
 * Have a look in the sysroot at include/bits/syscall.h to
 * determine the system call numbering.
 */

namespace wasm {
    WAVM_DEFINE_INTRINSIC_MODULE(env)

    WAVM_DEFINE_INTRINSIC_MODULE(wasi)

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "syscall", I32, syscall, I32 syscallNo, I32 argsPtr) {
        switch(syscallNo) {
            case 224:
                // gettid
                return executeSyscall(224, 0, 0, 0, 0, 0, 0, 0);
            default:
                faabric::util::getLogger()->error("Called unsupported syscall format {} {}", syscallNo, argsPtr);
                throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
        }
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "__syscall", I32, __syscall, I32 syscallNo, I32 argsPtr) {
        faabric::util::getLogger()->error("Called unsupported syscall format {} {}", syscallNo, argsPtr);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "__syscall0", I32, __syscall0, I32 syscallNo) {
        return executeSyscall(syscallNo, 0, 0, 0, 0, 0, 0, 0);
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "__syscall1", I32, __syscall1, I32 syscallNo,
                                   I32 a) {
        return executeSyscall(syscallNo, a, 0, 0, 0, 0, 0, 0);
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "__syscall2", I32, __syscall2, I32 syscallNo,
                                   I32 a, I32 b) {
        return executeSyscall(syscallNo, a, b, 0, 0, 0, 0, 0);
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "__syscall3", I32, __syscall3, I32 syscallNo,
                                   I32 a, I32 b, I32 c) {
        return executeSyscall(syscallNo, a, b, c, 0, 0, 0, 0);
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "__syscall4", I32, __syscall4, I32 syscallNo,
                                   I32 a, I32 b, I32 c, I32 d) {
        return executeSyscall(syscallNo, a, b, c, d, 0, 0, 0);
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "__syscall5", I32, __syscall5, I32 syscallNo,
                                   I32 a, I32 b, I32 c, I32 d, I32 e) {
        return executeSyscall(syscallNo, a, b, c, d, e, 0, 0);
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "__syscall6", I32, __syscall6, I32 syscallNo,
                                   I32 a, I32 b, I32 c, I32 d, I32 e, I32 f) {
        return executeSyscall(syscallNo, a, b, c, d, e, f, 0);
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "__syscall7", I32, __syscall7, I32 syscallNo,
                                   I32 a, I32 b, I32 c, I32 d, I32 e, I32 f, I32 g) {
        return executeSyscall(syscallNo, a, b, c, d, e, f, g);
    }

    I32 executeSyscall(int syscallNumber, int a, int b, int c, int d, int e, int f, int g) {
        switch (syscallNumber) {
            case 1:
                return s__exit(a, b);
            case 2:
                return s__fork();
            case 10:
                return s__unlink(a);
            case 33:
                return s__access(a, b);
            case 38:
                return s__rename(a, b);
            case 39:
                return s__mkdir(a, b);
            case 45:
                return s__brk(a);
            case 78:
                return s__gettimeofday(a, b);
            case 85:
                return s__readlink(a, b, c);
            case 90:
                return s__mmap(a, b, c, d, e, f);
            case 91:
                return s__munmap(a, b);
            case 102:
                return s__socketcall(a, b);
            case 125:
                return s__mprotect(a, b, c);
            case 162:
                return s__nanosleep(a, b);
            case 174:
                return s__sigaction(a, b, c);
            case 175:
                return s__rt_sigprocmask(a, b, c, d);
            case 186:
                return s__sigaltstack(a, b);
            case 192:
                // mmap2 is basically the same as mmap (difference is in the final argument which we ignore)
                return s__mmap(a, b, c, d, e, f);
            case 196:
                return s__lstat64(a, b);
            case 197:
                return s__fstat64(a, b);
            case 219:
                return s__madvise(a, b, c);
            case 220:
                return s__getdents64(a, b, c);
            case 223:
                return s__sbrk(a);
            case 224:
                return s__gettid();
            case 240:
                return s__futex(a, b, c, d, e, f);
            case 242:
                return s__sched_getaffinity(a, b, c);
            case 265:
                return s__clock_gettime(a, b);
            case 355:
                return s__getrandom(a, b, c);
            case 375:
                return s__membarrier(a);
            default:
                throw std::runtime_error("Unsupported system call: " + std::to_string(syscallNumber));
        }
    }

    void linkHook() {
        chainLink();
        dynlinkLink();
        envLink();
        faasmLink();
        ioLink();
        libcxxLink();
        mathsLink();
        memoryLink();
        messagesLink();
        mpiLink();
        networkLink();
        ompLink();
        processLink();
        rustLink();
        schedulingLink();
        signalsLink();
        threadsLink();
        timingLink();
    }
}