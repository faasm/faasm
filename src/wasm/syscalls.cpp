#include "wasm.h"

#include <atomic>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/uio.h>

#include <WAVM/Runtime/Runtime.h>
#include <WAVM/Runtime/RuntimeData.h>
#include <WAVM/Runtime/Intrinsics.h>

using namespace WAVM;

/**
 * USEFUL NOTES
 *
 * Any syscalls here must be included in the musl port at:
 *   - musl/arch/wasm32/syscall_arch.h and
 *   - musl/arch/wasm32/libc.imports
 *
 * Once implemented there, they will be referenced in the wasm files, and stubbed at runtime if they
 * don't exist in here.
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

    enum SocketCalls : U32 {
        socket = 1,
        bind,
        connect,
        listen,
        accept,
        getsockname,
        getpeername,
        socketpair,
        send,
        recv,
        sendto,
        recvfrom,
        shutdown,
        setsockopt,
        getsockopt,
        sendmsg,
        recvmsg,
        accept4,
        recvmmsg,
        sendmmsg,
    };

    /** Socket-related structs (see https://beej.us/guide/bgnet/html/multi/sockaddr_inman.html) */
    struct wasm_sockaddr {
        U16 sa_family;
        U8 sa_data[14];
    };

    struct wasm_in_addr {
        U32 s_addr;
    };

    struct wasm_sockaddr_in {
        U8 sin_family;
        U8 sin_port;
        struct  wasm_in_addr sin_addr;
        U8 sin_zero[8];
    };

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_socketcall", I32, __syscall_socketcall, I32 call, I32 argsPtr) {
        Runtime::MemoryInstance *memoryPtr = getModuleMemory();

        // NOTE
        // We don't want to support server-side socket syscalls as we expect functions
        // only to be clients

        switch (call) {
            case (SocketCalls::socket): {
                // TODO: limit/ track which sockets are in use?

                U32 *subCallArgs =  Runtime::memoryArrayPtr<U32>(memoryPtr, argsPtr, 3);
                U32 domain = subCallArgs[0];
                U32 type = subCallArgs[1];
                U32 protocol = subCallArgs[2];

                printf("SYSCALL - socket %i %i %i\n", domain, type, protocol);
                long sock = syscall(SYS_socket, domain, type, protocol);
                printf("Opened system socket %li\n", sock);

                return (I32) sock;
            }

            case(SocketCalls::bind):
                // Server-side
                printf("SYSCALL - bind %i %i\n", call, argsPtr);

                throwException(Runtime::Exception::calledUnimplementedIntrinsicType);

            case(SocketCalls::accept):
                // Server-side
                printf("SYSCALL - accept %i %i\n", call, argsPtr);

                throwException(Runtime::Exception::calledUnimplementedIntrinsicType);

            case(SocketCalls::connect): {
                U32 *subCallArgs =  Runtime::memoryArrayPtr<U32>(memoryPtr, argsPtr, 3);
                I32 sockfd = subCallArgs[0];

                I32 addrPtr = subCallArgs[1];
                auto addr =  Runtime::memoryRef<wasm_sockaddr>(getModuleMemory(), addrPtr);

                I32 addrLen = subCallArgs[2];

                printf("SYSCALL - connect - %i %i %i\n", sockfd, addrPtr, addrLen);
                printf("SYSCALL - connect sa_family: %i\n", addr.sa_family);
                printf("SYSCALL - connect data: %i %i %i\n", addr.sa_data[0], addr.sa_data[1], addr.sa_data[2]);

                return 0;
            }

            case(SocketCalls::listen): {
                // Server-side
                printf("SYSCALL - listen %i %i\n", call, argsPtr);

                throwException(Runtime::Exception::calledUnimplementedIntrinsicType);
            }

            case(SocketCalls::getsockname): {
                printf("SYSCALL - getsockname %i %i\n", call, argsPtr);
                return 0;
            }

            case(SocketCalls::getpeername): {
                printf("SYSCALL - getpeername %i %i\n", call, argsPtr);
                return 0;
            }

            case(SocketCalls::socketpair): {
                printf("SYSCALL - socketpair %i %i\n", call, argsPtr);
                return 0;
            }

            case(SocketCalls::send): {
                U32 *subCallArgs =  Runtime::memoryArrayPtr<U32>(memoryPtr, argsPtr, 4);
                I32 sockfd = subCallArgs[0];

                I32 msgPtr = subCallArgs[1];
                I32 len = subCallArgs[2];
                U8 *msgArray = Runtime::memoryArrayPtr<U8>(memoryPtr, msgPtr, len);

                I32 flags = subCallArgs[3];

                printf("SYSCALL - send %i %i %i %i \n", sockfd, msgPtr, len, flags);
                printf("SYSCALL - send message: %i %i %i \n", msgArray[0], msgArray[1], msgArray[2]);

                // Assume all bytes sent
                return len;
            }

            case(SocketCalls::recv): {
                printf("SYSCALL - recv %i %i\n", call, argsPtr);

                return 0;
            }

            case(SocketCalls::sendto): {
                U32 *subCallArgs =  Runtime::memoryArrayPtr<U32>(memoryPtr, argsPtr, 6);
                I32 sockfd = subCallArgs[0];

                I32 msgPtr = subCallArgs[1];
                I32 msgLen = subCallArgs[2];
                U8 *msgArray = Runtime::memoryArrayPtr<U8>(memoryPtr, msgPtr, msgLen);

                I32 flags = subCallArgs[3];

                I32 sockAddrPtr = subCallArgs[4];
                auto sockAddr =  Runtime::memoryRef<wasm_sockaddr>(getModuleMemory(), sockAddrPtr);

                I32 sockAddrLen = subCallArgs[5];

                printf("SYSCALL - sendto - %i %i %i %i %i %i\n", sockfd, msgPtr, msgLen, flags, sockAddrPtr, sockAddrLen);
                printf("SYSCALL - sendto message: %i %i %i\n", msgArray[0], msgArray[1], msgArray[2]);
                printf("SYSCALL - sendto sockAddr.sa_family: %i\n", sockAddr.sa_family);
                printf("SYSCALL - sendto sockAddr: %i %i %i\n", sockAddr.sa_data[0], sockAddr.sa_data[1], sockAddr.sa_data[2]);

                return msgLen;
            }

            case(SocketCalls::recvfrom): {
                printf("SYSCALL - recvfrom %i %i\n", call, argsPtr);
                return 0;
            }
            case(SocketCalls::shutdown): {
                printf("SYSCALL - shutdown %i %i\n", call, argsPtr);
                return 0;
            }

            case(SocketCalls::setsockopt): {
                printf("SYSCALL - setsockopt %i %i\n", call, argsPtr);
                return 0;
            }
            case(SocketCalls::getsockopt): {
                printf("SYSCALL - getsockopt %i %i\n", call, argsPtr);
                return 0;
            }

            case(SocketCalls::sendmsg): {
                printf("SYSCALL - sendmsg %i %i\n", call, argsPtr);
                return 0;
            }

            case(SocketCalls::recvmsg): {
                printf("SYSCALL - recvmsg %i %i\n", call, argsPtr);
                return 0;
            }

            case(SocketCalls::accept4): {
                printf("SYSCALL - accept4 %i %i\n", call, argsPtr);
                return 0;
            }

            case(SocketCalls::recvmmsg): {
                printf("SYSCALL - recvmmsg %i %i\n", call, argsPtr);
                return 0;
            }

            case(SocketCalls::sendmmsg): {
                printf("SYSCALL - sendmmsg %i %i\n", call, argsPtr);
                return 0;
            }

            default: {
                printf("Unrecognised socketcall %i\n", call);
                return 0;
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
        // Dumping ground for unimplemented calls
        // See wasm.js file in toolchain (toolchain/wasm-install/lib/wasm.js)
        switch(a) {
            case(6): {
                printf("SYSCALL - close %i %i\n", a, b);
                break;
            }
            default: {
                printf("SYSCALL - generic (syscall) %i %i\n", a, b);
                break;
            }
        }

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