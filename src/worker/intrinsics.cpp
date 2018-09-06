#include "intrinsics.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>

#include <atomic>

#include "Runtime/RuntimeData.h"
#include "Runtime/Intrinsics.h"

using namespace Runtime;

namespace intrinsics {
    /**
     * socketcall
     */
    DEFINE_INTRINSIC_FUNCTION_WITH_MEM_AND_TABLE(faasm, "___syscall102", I32, ___syscall102, I32 syscallNo,
                                                 I32 argsPtr) {
        MemoryInstance *memory = Runtime::getMemoryFromRuntimeData(contextRuntimeData, defaultMemoryId.id);
        U32 *args = memoryArrayPtr<U32>(memory, argsPtr, 2);
        U32 call = args[0];
        U32 callArgsPtr = args[1];

        // See mapping in Emscripten repo to see socketcall codes
        // https://github.com/kripken/emscripten/blob/master/system/lib/libc/musl/src/internal/syscall.h#L227
        switch (call) {
            case (1): { // socket
                U32 *subCallArgs = memoryArrayPtr<U32>(memory, callArgsPtr, 3);
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

    DEFINE_INTRINSIC_FUNCTION(faasm, "___syscall122", I32, ___syscall122, I32 a, I32 b) {
        return 0;
    }

    DEFINE_INTRINSIC_FUNCTION_WITH_MEM_AND_TABLE(faasm, "___syscall142", I32, ___syscall142, I32 syscallNo,
                                                 I32 argsPtr) {
//        MemoryInstance* memory = getMemoryFromRuntimeData(contextRuntimeData,defaultMemoryId.id);
//        U32* args = memoryArrayPtr<U32>(memory, argsPtr, 2);

        return 0;
    }

    DEFINE_INTRINSIC_FUNCTION_WITH_MEM_AND_TABLE(faasm, "_gethostbyname", I32, _gethostbyname, I32 hostnamePtr) {
        MemoryInstance *memory = getMemoryFromRuntimeData(contextRuntimeData, defaultMemoryId.id);
        auto hostname = &memoryRef<char>(memory, (Uptr) hostnamePtr);

        printf("INTRINSIC - gethostbyname %s\n", hostname);

        return 0;
    }

    DEFINE_INTRINSIC_FUNCTION(faasm, "___syscall195", I32, ___syscall195, I32 a, I32 b) {
        return 0;
    }

    DEFINE_INTRINSIC_FUNCTION(faasm, "___syscall197", I32, ___syscall197, I32 a, I32 b) {
        return 0;
    }

    DEFINE_INTRINSIC_FUNCTION(faasm, "___syscall201", I32, ___syscall201, I32 a, I32 b) {
        return 0;
    }

    DEFINE_INTRINSIC_FUNCTION(faasm, "___syscall221", I32, ___syscall221, I32 a, I32 b) {
        return 0;
    }

    DEFINE_INTRINSIC_FUNCTION(faasm, "___syscall3", I32, ___syscall3, I32 a, I32 b) {
        return 0;
    }

    DEFINE_INTRINSIC_FUNCTION(faasm, "___syscall33", I32, ___syscall33, I32 a, I32 b) {
        return 0;
    }

    DEFINE_INTRINSIC_FUNCTION(faasm, "___syscall4", I32, ___syscall4, I32 a, I32 b) {
        return 0;
    }

    DEFINE_INTRINSIC_FUNCTION(faasm, "___syscall5", I32, ___syscall5, I32 a, I32 b) {
        return 0;
    }

/** Struct to fake 32-bit time in wasm modules */
    struct wasm_timespec {
        I32 tv_sec;
        I32 tv_nsec;
    };

    DEFINE_INTRINSIC_FUNCTION_WITH_MEM_AND_TABLE(faasm, "_clock_gettime", I32, _clock_gettime, I32 clockId,
                                                 I32 resultAddress) {
        printf("INTRINSIC - _clock_gettime\n");

        // Get module's default memory
        MemoryInstance *memory = getMemoryFromRuntimeData(contextRuntimeData, defaultMemoryId.id);
        auto result = memoryRef<wasm_timespec>(memory, (Uptr) resultAddress);

        // Fake a clock incrementing by 1 with each call
        static std::atomic<U64> fakeClock;
        const U64 currentClock = fakeClock;

        result.tv_sec = I32(currentClock / 1000000000);
        result.tv_nsec = I32(currentClock % 1000000000);
        ++fakeClock;

        return 0;
    }

    DEFINE_INTRINSIC_FUNCTION(faasm, "_getaddrinfo", I32, _getaddrinfo, I32 a, I32 b, I32 c, I32 d) {
        return 0;
    }

    DEFINE_INTRINSIC_FUNCTION(faasm, "_getpwuid", I32, _getpwuid, I32 a) {
        return 0;
    }

    DEFINE_INTRINSIC_FUNCTION(faasm, "_gettimeofday", I32, _gettimeofday, I32 a, I32 b) {
        return 0;
    }

    DEFINE_INTRINSIC_FUNCTION(faasm, "_gmtime_r", I32, _gmtime_r, I32 a, I32 b) {
        return 0;
    }

}