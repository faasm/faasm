#include "WasmModule.h"
#include "syscalls.h"

#include <sys/time.h>

#include <WAVM/Runtime/Intrinsics.h>

namespace wasm {


    //TODO - make timing functions more secure
    I32 s__clock_gettime(I32 clockId, I32 timespecPtr) {
        util::getLogger()->debug("S - clock_gettime - {} {}", clockId, timespecPtr);

        timespec ts{};
        int retVal = clock_gettime(clockId, &ts);
        if (retVal == -1) {
            int systemErrno = errno;
            util::getLogger()->error("Clock type not supported - {} ({})\n", systemErrno, strerror(systemErrno));
            return -systemErrno;
        }

        auto result = &Runtime::memoryRef<wasm_timespec>(getExecutingModule()->defaultMemory, (Uptr) timespecPtr);
        result->tv_sec = I64(ts.tv_sec);
        result->tv_nsec = I32(ts.tv_nsec);

        return 0;
    }

    /**
     * As specified in the gettimeofday man page, use of the timezone struct is obsolete and hence not supported here
     */
    I32 s__gettimeofday(int tvPtr, int tzPtr) {
        util::getLogger()->debug("S - gettimeofday - {} {}", tvPtr, tzPtr);

        timeval tv{};
        gettimeofday(&tv, nullptr);

        auto result = &Runtime::memoryRef<wasm_timeval>(getExecutingModule()->defaultMemory, (Uptr) tvPtr);
        result->tv_sec = I32(tv.tv_sec);
        result->tv_usec = I32(tv.tv_usec);

        return 0;
    }

    /**
     * Allow sleep for now
     */
    I32 s__nanosleep(I32 reqPtr, I32 remPtr) {
        util::getLogger()->debug("S - nanosleep - {} {}", reqPtr, remPtr);

        auto request = &Runtime::memoryRef<wasm_timespec>(getExecutingModule()->defaultMemory, (Uptr) reqPtr);

        // TODO - is this ok? Should we allow?
        // Round up
        sleep(request->tv_sec + 1);

        return 0;
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "utime", I32, s__utime, I32 a, I32 b) {
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(wasi, "clock_time_get", I32, wasi_clock_time_get, I32 a, I64 b,
                                   I32 c) { throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic); }

    WAVM_DEFINE_INTRINSIC_FUNCTION(wasi, "clock_res_get", I32, wasi_clock_res_get, I32 a, I32 b) {
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    void timingLink() {

    }
}