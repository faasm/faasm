#include "WasmModule.h"
#include "syscalls.h"

#include <util/bytes.h>
#include <util/config.h>

#include <sys/time.h>

#include <WAVM/Runtime/Runtime.h>
#include <WAVM/Runtime/RuntimeData.h>
#include <WAVM/Runtime/Intrinsics.h>

namespace wasm {
    I32 s__getpid() {
        return FAKE_PID;
    }

    I32 s__getuid32() {
        util::getLogger()->debug("S - getuid32");
        return FAKE_UID;
    }

    I32 s__getgid32() {
        util::getLogger()->debug("S - getgid32");
        return FAKE_GID;
    }

    I32 s__geteuid32() {
        util::getLogger()->debug("S - geteuid32");
        return FAKE_UID;
    }

    I32 s__getegid32() {
        util::getLogger()->debug("S - getegid32");
        return FAKE_GID;
    }

    I32 s__exit(I32 a, I32 b) {
        util::getLogger()->debug("S - exit - {} {}", a, b);
        throw (wasm::WasmExitException(a));
    }

    DEFINE_INTRINSIC_FUNCTION(env, "confstr", I32, confstr, I32 a, I32 b, I32 c) {
        util::getLogger()->debug("S - confstr - {} {} {}", a, b, c);

        // Return zero as if no confstr variables have a value set
        return 0;
    }

    DEFINE_INTRINSIC_FUNCTION(env, "getpwuid", I32, getpwuid, I32 uid) {
        util::getLogger()->debug("S - _getpwuid - {}", uid);

        WasmModule *module = getExecutingModule();
        Runtime::Memory *memoryPtr = module->defaultMemory;

        // Provision a new segment of memory big enough to hold the strings and the struct
        size_t memSize = strlen(FAKE_NAME) + strlen(FAKE_PASSWORD) + strlen(FAKE_HOME) + sizeof(wasm_passwd);
        U32 newMem = module->mmap(memSize);
        char *hostNewMem = Runtime::memoryArrayPtr<char>(memoryPtr, newMem, memSize);

        // Copy the strings into place
        std::strcpy(hostNewMem, FAKE_NAME);
        std::strcpy(hostNewMem + strlen(FAKE_NAME), FAKE_PASSWORD);
        std::strcpy(hostNewMem + strlen(FAKE_NAME) + strlen(FAKE_PASSWORD), FAKE_HOME);

        // Get a pointer to it
        I32 structOffset = (I32) memSize - sizeof(wasm_passwd);
        wasm_passwd *wasmPasswd = &Runtime::memoryRef<wasm_passwd>(memoryPtr, structOffset);

        wasmPasswd->pw_name = newMem;
        wasmPasswd->pw_passwd = newMem + strlen(FAKE_NAME);
        wasmPasswd->pw_uid = FAKE_UID;
        wasmPasswd->pw_gid = FAKE_GID;
        wasmPasswd->pw_dir = newMem + strlen(FAKE_NAME) + strlen(FAKE_PASSWORD);

        return structOffset;
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__faasm_read_config", void, __faasm_read_config, I32 varPtr, I32 buffer) {
        WasmModule *module = getExecutingModule();
        Runtime::Memory *memoryPtr = module->defaultMemory;
        char *varName = &Runtime::memoryRef<char>(memoryPtr, (Uptr) varPtr);

        util::SystemConfig &conf = util::getSystemConfig();

        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->debug("S - faasm_read_config - {}", varName);

        const char *value = "";

        if (strcmp(varName, "FULL_ASYNC") == 0) {
            if (conf.fullAsync == 1) {
                value = "1";
            } else {
                value = "0";
            }
        } else if (strcmp(varName, "FULL_SYNC") == 0) {
            if (conf.fullSync == 1) {
                value = "1";
            } else {
                value = "0";
            }
        }

        char* resultBuffer = &Runtime::memoryRef<char>(memoryPtr, (Uptr) buffer);
        std::strcpy(resultBuffer, value);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "abort", void, abort) {
        util::getLogger()->debug("S - abort");
        throw (wasm::WasmExitException(0));
    }

    DEFINE_INTRINSIC_FUNCTION(env, "exit", void, exit, I32 a) {
        util::getLogger()->debug("S - exit - {}", a);
        throw (wasm::WasmExitException(a));
    }

    DEFINE_INTRINSIC_FUNCTION(env, "_Exit", void, _Exit, I32 a) {
        util::getLogger()->debug("S - _Exit - {}", a);
        throw (wasm::WasmExitException(a));
    }

    /**
     * Allowing straight-through access to sysconf my not be wise. Should revisit this.
     */
    DEFINE_INTRINSIC_FUNCTION(env, "sysconf", I32, _sysconf, I32 a) {
        util::getLogger()->debug("S - _sysconf - {}", a);

        util::SystemConfig &conf = util::getSystemConfig();
        if (conf.unsafeMode == "on") {
            return sysconf(a);
        } else {
            throwException(Runtime::ExceptionTypes::calledAbort);
        }
    }

    // ------------------------
    // Timing - supported
    // ------------------------

    //TODO - make timing functions more secure
    I32 s__clock_gettime(I32 clockId, I32 timespecPtr) {
        util::getLogger()->debug("S - clock_gettime - {} {}", clockId, timespecPtr);

        timespec ts{};
        clock_gettime(CLOCK_MONOTONIC, &ts);

        auto result = &Runtime::memoryRef<wasm_timespec>(getExecutingModule()->defaultMemory, (Uptr) timespecPtr);
        result->tv_sec = I32(ts.tv_sec);
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
}