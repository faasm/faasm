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
 * Any structs passed as arguments must be re-implemented here with the following mappings
 * (respecting signed/ unsigned):
 *
 * short = I16/U16
 * long = I32/U32
 * int = I32/U32
 * char = U8
 * pointers = Uptr
 * size_t = I32
 *
 */

namespace wasm {
    static Intrinsics::Module envModule;

    Intrinsics::Module &getIntrinsicModule_env() {
        return envModule;
    }

    /**
     * This is the interface Emscripten provides with the LLVM wasm backend.
     */
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

    I32 executeSyscall(int syscallNumber, int a, int b, int c, int d, int e, int f) {
        switch(syscallNumber) {
            case 1:
                return s__exit(a, b);
            case 3:
                return s__read(a, b, c);
            case 5:
                return s__open(a, b, c);
            case 20:
                return s__getpid();
            case 41:
                return s__dup(a);
            case 85:
                return s__readlink(a, b, c);
            case 183:
                return s__getcwd(a, b);
            case 199:
                return s__getuid32();
            case 200:
                return s__getgid32();
            case 201:
                return s__geteuid32();
            case 202:
                return s__getegid32();
            case 220:
                return s__getdents64(a, b, c);
            case 221:
                return s__fcntl64(a, b, c);
            default:
                throw std::runtime_error("Unsupported system call: " + std::to_string(syscallNumber));
        }
    }

}