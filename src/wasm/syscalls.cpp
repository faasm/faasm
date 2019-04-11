#include "WasmModule.h"
#include "syscalls.h"

#include <util/logging.h>

#include <stdio.h>

#include <WAVM/Runtime/Runtime.h>
#include <WAVM/Runtime/RuntimeData.h>
#include <WAVM/Runtime/Intrinsics.h>
#include <WAVM/Inline/FloatComponents.h>

using namespace WAVM;

/**
 * USEFUL NOTES
 *
 * The wasm syscall interface varies across toolchains. To work out what's going on
 * locate the musl port you're compiling against (e.g. in the emscripten sysroot), and
 * look at
 *   - musl//src/internal/syscall.h
 *   - musl/arch/XXX/syscall_arch.h
 *   - musl/arch/XXX/libc.imports
 *
 * where XXX is either wasm32 or emscripten.
 *
 * Syscall numbers can be found in musl/arch/XXX/bits/syscall.h(.in)
 *
 * The full i386 table can also give hints for higher numbers:
 * https://github.com/torvalds/linux/blob/master/arch/x86/entry/syscalls/syscall_32.tbl
 */

namespace wasm {
    static Intrinsics::Module envModule;
    static Intrinsics::Module GOTModule;

    Intrinsics::Module &getIntrinsicModule_env() {
        return envModule;
    }

    Intrinsics::Module &getIntrinsicModule_GOT() {
        return GOTModule;
    }

    /**
     * This is the interface Emscripten provides with the LLVM wasm backend.
     */
    DEFINE_INTRINSIC_FUNCTION(env, "__syscall", I32, __syscall, I32 syscallNo,
                              I32 argsPtr) {
        switch (syscallNo) {
            default:
                util::getLogger()->error("Called unsupported syscall format {} {}", syscallNo, argsPtr);
                throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
        }
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall0", I32, __syscall0, I32 syscallNo) {
        return executeSyscall(syscallNo, 0, 0, 0, 0, 0, 0, 0);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall1", I32, __syscall1, I32 syscallNo,
                              I32 a) {
        return executeSyscall(syscallNo, a, 0, 0, 0, 0, 0, 0);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall2", I32, __syscall2, I32 syscallNo,
                              I32 a, I32 b) {
        return executeSyscall(syscallNo, a, b, 0, 0, 0, 0, 0);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall3", I32, __syscall3, I32 syscallNo,
                              I32 a, I32 b, I32 c) {
        return executeSyscall(syscallNo, a, b, c, 0, 0, 0, 0);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall4", I32, __syscall4, I32 syscallNo,
                              I32 a, I32 b, I32 c, I32 d) {
        return executeSyscall(syscallNo, a, b, c, d, 0, 0, 0);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall5", I32, __syscall5, I32 syscallNo,
                              I32 a, I32 b, I32 c, I32 d, I32 e) {
        return executeSyscall(syscallNo, a, b, c, d, e, 0, 0);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall6", I32, __syscall6, I32 syscallNo,
                              I32 a, I32 b, I32 c, I32 d, I32 e, I32 f) {
        return executeSyscall(syscallNo, a, b, c, d, e, f, 0);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall7", I32, __syscall7, I32 syscallNo,
                              I32 a, I32 b, I32 c, I32 d, I32 e, I32 f, I32 g) {
        return executeSyscall(syscallNo, a, b, c, d, e, f, g);
    }

    I32 executeSyscall(int syscallNumber, int a, int b, int c, int d, int e, int f, int g) {
        switch (syscallNumber) {
            case 1:
                return s__exit(a, b);
            case 2:
                return s__fork();
            case 3:
                return s__read(a, b, c);
            case 4:
                return s__write(a, b, c);
            case 5:
                return s__open(a, b, c);
            case 6:
                return s__close(a);
            case 10:
                return s__unlink(a);
            case 20:
                return s__getpid();
            case 38:
                return s__rename(a, b);
            case 39:
                return s__mkdir(a, b);
            case 41:
                return s__dup(a);
            case 45:
                return s__brk(a);
            case 48:
                return s__signal(a, b);
            case 54:
                return s__ioctl(a, b, c, d, e, f);
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
            case 140:
                return s__llseek(a, b, c, d, e);
            case 145:
                return s__readv(a, b, c);
            case 146:
                return s__writev(a, b, c);
            case 168:
                return s__poll(a, b, c);
            case 174:
                return s__sigaction(a, b, c);
            case 175:
                return s__rt_sigprocmask(a, b, c, d);
            case 183:
                return s__getcwd(a, b);
            case 186:
                return s__sigaltstack(a, b);
            case 192:
                // mmap2 is basically the same as mmap (difference is in the final argument which we ignore)
                return s__mmap(a, b, c, d, e, f);
            case 195:
                return s__stat64(a, b);
            case 196:
                return s__lstat64(a, b);
            case 197:
                return s__fstat64(a, b);
            case 199:
                return s__getuid32();
            case 200:
                return s__getgid32();
            case 201:
                return s__geteuid32();
            case 202:
                return s__getegid32();
            case 219:
                return s__madvise(a, b, c);
            case 220:
                return s__getdents64(a, b, c);
            case 221:
                return s__fcntl64(a, b, c);
            case 240:
                return s__futex(a, b, c, d, e, f);
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

    void dynlinkLinkLink() {
        dynlinkLink();
    }
}