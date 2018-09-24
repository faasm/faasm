#include "wasm.h"

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/uio.h>

#include <atomic>

#include <WAVM/Runtime/Runtime.h>
#include <WAVM/Runtime/RuntimeData.h>
#include <WAVM/Runtime/Intrinsics.h>

using namespace WAVM;

namespace wasm {
    DEFINE_INTRINSIC_MODULE(env)

    // ------------------------
    // I/O
    // ------------------------

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_writev", I32, __syscall_writev, I32 fd, I32 iov, I32 iovcnt) {
        printf("SYSCALL - writev %i %i %i\n", fd, iov, iovcnt);

        struct iovec* native_iovec = new(alloca(sizeof(iovec) * iovcnt)) struct iovec[iovcnt];
        for(U32 i = 0; i < iovcnt; i++)
        {
            Runtime::MemoryInstance *memoryPtr = getModuleMemory();
            U32 base = Runtime::memoryRef<U32>(memoryPtr, iov + i * 8);
            U32 len = Runtime::memoryRef<U32>(memoryPtr, iov + i * 8 + 4);

            native_iovec[i].iov_base = Runtime::memoryArrayPtr<U8>(memoryPtr, base, len);
            native_iovec[i].iov_len = len;
        }
        Iptr count = writev(fileno(stdout), native_iovec, iovcnt);

        return (I32) count;
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_readv", I32, __syscall_readv,
                              I32 a, I32 b, I32 c) {
        printf("SYSCALL - readv %i %i %i \n", a, b, c);
        throwException(Runtime::Exception::calledUnimplementedIntrinsicType);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_read", I32, __syscall_read,
                              I32 a, I32 b, I32 c) {
        printf("SYSCALL - read %i %i %i \n", a, b, c);
        throwException(Runtime::Exception::calledUnimplementedIntrinsicType);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "puts", I32, puts, I32 strPtr) {
        Runtime::MemoryInstance *memoryPtr = getModuleMemory();
        char* string = &Runtime::memoryRef<char>(memoryPtr, (Uptr)strPtr);

        printf("puts - %s\n", string);

        return 0;
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_ioctl", I32, __syscall_ioctl,
                                                 I32 a, I32 b, I32 c, I32 d, I32 e, I32 f) {
        printf("SYSCALL - ioctl %i %i %i %i %i %i\n", a, b, c, d, e, f);

        return 0;
    }

    DEFINE_INTRINSIC_FUNCTION(env, "ioctl", I32, ioctl, I32 a, I32 b, I32 c) {
        printf("INTRINSIC - ioctl %i %i %i\n", a, b, c);

        return 0;
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_poll", I32, __syscall_poll, I32 a, I32 b, I32 c) {
        printf("SYSCALL - poll %i %i %i\n", a, b, c);
        throwException(Runtime::Exception::calledUnimplementedIntrinsicType);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_open", I32, __syscall_open, I32 a, I32 b, I32 c) {
        printf("SYSCALL - open %i %i %i\n", a, b, c);
        throwException(Runtime::Exception::calledUnimplementedIntrinsicType);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_llseek", I32, __syscall_llseek, I32 a, I32 b, I32 c, I32 d, I32 e) {
        printf("SYSCALL - llseek %i %i %i %i %i\n", a, b, c, d, e);
        throwException(Runtime::Exception::calledUnimplementedIntrinsicType);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_close", I32, __syscall_close, I32 a) {
        printf("SYSCALL - close %i\n", a);
        throwException(Runtime::Exception::calledUnimplementedIntrinsicType);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_futex", I32, __syscall_futex,
                              I32 a, I32 b, I32 c, I32 d, I32 e, I32 f) {
        printf("SYSCALL - futex %i %i %i %i %i %i\n", a, b, c, d, e, f);
        throwException(Runtime::Exception::calledUnimplementedIntrinsicType);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_fcntl64", I32, __syscall_fcntl64,
                              I32 a, I32 b, I32 c) {
        printf("SYSCALL - fcntl64 %i %i %i\n", a, b, c);
        throwException(Runtime::Exception::calledUnimplementedIntrinsicType);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_fstat64", I32, __syscall_fstat64, I32 a, I32 b) {
        printf("SYSCALL - fstat64 %i %i \n", a, b);
        throwException(Runtime::Exception::calledUnimplementedIntrinsicType);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_stat64", I32, __syscall_stat64, I32 a, I32 b) {
        printf("SYSCALL - stat64 %i %i \n", a, b);
        throwException(Runtime::Exception::calledUnimplementedIntrinsicType);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_access", I32, __syscall_access, I32 a, I32 b) {
        printf("SYSCALL - access %i %i \n", a, b);
        throwException(Runtime::Exception::calledUnimplementedIntrinsicType);
    }

    // ------------------------
    // Sockets/ network
    // ------------------------

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_socketcall", I32, __syscall_socketcall, I32 syscallNo, I32 argsPtr) {

        Runtime::MemoryInstance *memoryPtr = getModuleMemory();
        U32 *args = Runtime::memoryArrayPtr<U32>(memoryPtr, argsPtr, 2);
        U32 call = args[0];
        U32 callArgsPtr = args[1];

        switch (call) {
            case (1): { // socket
                U32 *subCallArgs =  Runtime::memoryArrayPtr<U32>(memoryPtr, callArgsPtr, 3);
                U32 domain = subCallArgs[0];
                U32 type = subCallArgs[1];
                U32 protocol = subCallArgs[2];

                printf("SYSCALL - socket %i %i %i\n", domain, type, protocol);
                long sock = syscall(SYS_socket, domain, type, protocol);
                printf("Opened system socket %li\n", sock);

                return (I32) sock;
            }
            default: {
                printf("Unrecognised socketcall %i\n", call);
            }
        }

        return 0;
    }

    DEFINE_INTRINSIC_FUNCTION(env, "_gethostbyname", I32, _gethostbyname, I32 hostnamePtr) {
        auto hostname = & Runtime::memoryRef<char>(getModuleMemory(), (Uptr) hostnamePtr);

        printf("INTRINSIC - gethostbyname %s\n", hostname);

        return 0;
    }
    
    // ------------------------
    // Timing
    // ------------------------

    /** Struct to fake 32-bit time in wasm modules */
    struct wasm_timespec {
        I32 tv_sec;
        I32 tv_nsec;
    };

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_clock_gettime", I32, __syscall_clock_gettime,
            I32 clockId, I32 resultAddress) {
        printf("INTRINSIC - clock_gettime %i %i\n", clockId, resultAddress);

        auto result =  Runtime::memoryRef<wasm_timespec>(getModuleMemory(), (Uptr) resultAddress);

        // Fake a clock incrementing by 1 with each call
        static std::atomic<U64> fakeClock;
        const U64 currentClock = fakeClock;

        result.tv_sec = I32(currentClock / 1000000000);
        result.tv_nsec = I32(currentClock % 1000000000);
        ++fakeClock;

        return 0;
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_gettimeofday", I32, __syscall_gettimeofday,
                              I32 a, I32 b) {
        printf("SYSCALL - gettimeofday %i %i\n", a, b);
        throwException(Runtime::Exception::calledUnimplementedIntrinsicType);
    }

    // ------------------------
    // Misc
    // ------------------------

    DEFINE_INTRINSIC_FUNCTION(env, "__unsupported_syscall", I32, __unsupported_syscall,
            I32 a, I32 b, I32 c, I32 d, I32 e, I32 f, I32 g) {
        printf("SYSCALL - UNSUPPORTED %i %i %i %i %i %i %i \n", a, b, c, d, e, f, g);
        throwException(Runtime::Exception::calledUnimplementedIntrinsicType);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall", I32, __syscall, I32 a, I32 b) {
        printf("SYSCALL - generic (syscall) %i %i\n", a, b);
        throwException(Runtime::Exception::calledUnimplementedIntrinsicType);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_exit_group", I32, __syscall_exit_group, I32 a) {
        printf("SYSCALL - exit_group %i\n", a);
        throwException(Runtime::Exception::calledUnimplementedIntrinsicType);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_exit", I32, __syscall_exit, I32 a) {
        printf("SYSCALL - exit %i\n", a);
        throwException(Runtime::Exception::calledUnimplementedIntrinsicType);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_gettid", I32, __syscall_gettid, I32 a) {
        printf("SYSCALL - gettid %i\n", a);
        throwException(Runtime::Exception::calledUnimplementedIntrinsicType);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_tkill", I32, __syscall_tkill, I32 a, I32 b) {
        printf("SYSCALL - tkill %i %i\n", a, b);
        throwException(Runtime::Exception::calledUnimplementedIntrinsicType);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_rt_sigprocmask", I32, __syscall_rt_sigprocmask, I32 a, I32 b, I32 c) {
        printf("SYSCALL - rt_sigprocmask %i %i %i\n", a, b, c);
        throwException(Runtime::Exception::calledUnimplementedIntrinsicType);
    }

    // ------------------------
    // Memory
    // ------------------------

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_mmap", I32, __syscall_mmap,
            I32 a, I32 b, I32 c, I32 d, I32 e, I32 f) {
        printf("SYSYCALL - mmap %i %i %i %i %i %i\n", a, b, c, d, e, f);
        throwException(Runtime::Exception::calledUnimplementedIntrinsicType);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_mremap", I32, __syscall_mremap,
                              I32 a, I32 b, I32 c, I32 d, I32 e) {
        printf("SYSYCALL - mremap %i %i %i %i %i\n", a, b, c, d, e);
        throwException(Runtime::Exception::calledUnimplementedIntrinsicType);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_madvise", I32, __syscall_madvise, I32 a, I32 b, I32 c) {
        printf("SYSYCALL - madvise %i %i %i\n", a, b, c);
        throwException(Runtime::Exception::calledUnimplementedIntrinsicType);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_munmap", I32, __syscall_munmap, I32 a, I32 b) {
        printf("SYSYCALL - munmap %i %i\n", a, b);
        throwException(Runtime::Exception::calledUnimplementedIntrinsicType);
    }
}