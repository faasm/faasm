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
        return FAKE_UID;
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


    DEFINE_INTRINSIC_FUNCTION(env, "getenv", I32, getenv, I32 varPtr) {
        WasmModule *module = getExecutingModule();
        Runtime::Memory *memoryPtr = module->defaultMemory;
        char *varName = &Runtime::memoryRef<char>(memoryPtr, (Uptr) varPtr);

        util::SystemConfig &conf = util::getSystemConfig();

        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->debug("S - _getenv - {}", varName);

        const char *value = "";

        // Special cases
        if (strcmp(varName, "PYTHONHASHSEED") == 0) {
            value = "0";
        } else if (strcmp(varName, "PYTHONVERBOSE") == 0) {
            value = "on";
        } else if (strcmp(varName, "PYTHONHOME") == 0) {
            value = "/";
        } else if (strcmp(varName, "PYTHONPATH") == 0) {
            value = "/";
        } else if (strcmp(varName, "OPENBLAS_MAIN_FREE") == 0) {
            value = "1";
        } else if (strcmp(varName, "GOTOBLAS_MAIN_FREE") == 0) {
            value = "1";
        } else if (strcmp(varName, "LC_CTYPE") == 0) {
            value = "en_GB.UTF-8";
//        } else if (strcmp(varName, "LANG") == 0) {
//            value = "en_GB.UTF-8";
//        } else if (strcmp(varName, "LANGUAGE") == 0) {
//            value = "en_GB:en";
        } else if (strcmp(varName, "FULL_ASYNC") == 0) {
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

        // Create the region of memory
        size_t nBytes = strlen(value) + 1;
        U32 result = module->mmap(nBytes);

        // Copy value into place
        char *hostPtr = Runtime::memoryArrayPtr<char>(memoryPtr, (Uptr) result, nBytes);
        std::strcpy(hostPtr, value);

        return result;
    }

    DEFINE_INTRINSIC_FUNCTION(env, "setenv", I32, setenv, I32 varPtr, I32 valPtr, I32 overwrite) {
        const std::string varName = getStringFromWasm(varPtr);
        const std::string value = getStringFromWasm(valPtr);

        util::getLogger()->debug("S - setenv {} {} {}", varName, value, overwrite);

        return 0;
    }

    DEFINE_INTRINSIC_FUNCTION(env, "putenv", I32, putenv, I32 varStringPtr) {
        const std::string varString = getStringFromWasm(varStringPtr);

        util::getLogger()->debug("S - putenv {}", varString);

        return 0;
    }

    DEFINE_INTRINSIC_FUNCTION(env, "unsetenv", I32, unsetenv, I32 varPtr) {
        Runtime::Memory *memoryPtr = getExecutingModule()->defaultMemory;
        char *varName = &Runtime::memoryRef<char>(memoryPtr, (Uptr) varPtr);

        util::getLogger()->debug("S - _unsetenv {} {} {}", varName);

        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "abort", void, abort) {
        util::getLogger()->debug("S - abort");
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
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
    DEFINE_INTRINSIC_FUNCTION(env, "clock_gettime", I32, clock_gettime, I32 clockId, I32 resultAddress) {
        util::getLogger()->debug("S - clock_gettime - {} {}", clockId, resultAddress);

        timespec ts{};
        clock_gettime(CLOCK_MONOTONIC, &ts);

        auto result = &Runtime::memoryRef<wasm_timespec>(getExecutingModule()->defaultMemory, (Uptr) resultAddress);
        result->tv_sec = I32(ts.tv_sec);
        result->tv_nsec = I32(ts.tv_nsec);

        return 0;
    }

    /**
     * As specified in the gettimeofday man page, use of the timezone struct is obsolete and hence not supported here
     */
    DEFINE_INTRINSIC_FUNCTION(env, "gettimeofday", I32, gettimeofday, I32 timevalPtr, I32 tzPtr) {
        util::getLogger()->debug("S - gettimeofday - {} {}", timevalPtr, tzPtr);

        timeval tv{};
        gettimeofday(&tv, nullptr);

        auto result = &Runtime::memoryRef<wasm_timeval>(getExecutingModule()->defaultMemory, (Uptr) timevalPtr);
        result->tv_sec = I32(tv.tv_sec);
        result->tv_usec = I32(tv.tv_usec);

        return 0;
    }
}