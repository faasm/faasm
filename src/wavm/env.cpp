#include "WAVMWasmModule.h"
#include "syscalls.h"

#include <faabric/util/bytes.h>
#include <faabric/util/config.h>
#include <faabric/util/macros.h>
#include <wasm/WasmModule.h>

#include <sys/random.h>

#include <WAVM/Runtime/Intrinsics.h>
#include <WAVM/Runtime/Runtime.h>
#include <WAVM/WASI/WASIABI.h>

using namespace WAVM;

namespace wasm {

WAVM_DEFINE_INTRINSIC_FUNCTION(wasi,
                               "args_sizes_get",
                               I32,
                               wasi_args_sizes_get,
                               I32 argcPtr,
                               I32 argvBufSize)
{
    faabric::util::getLogger()->debug(
      "S - args_sizes_get - {} {}", argcPtr, argvBufSize);
    WAVMWasmModule* module = getExecutingWAVMModule();

    Runtime::memoryRef<U32>(module->defaultMemory, argcPtr) = module->getArgc();
    Runtime::memoryRef<U32>(module->defaultMemory, argvBufSize) =
      module->getArgvBufferSize();

    return __WASI_ESUCCESS;
}

WAVM_DEFINE_INTRINSIC_FUNCTION(wasi,
                               "args_get",
                               I32,
                               wasi_args_get,
                               I32 argvPtr,
                               I32 argvBufPtr)
{
    faabric::util::getLogger()->debug(
      "S - args_get - {} {}", argvPtr, argvBufPtr);
    WAVMWasmModule* module = getExecutingWAVMModule();
    module->writeArgvToMemory(argvPtr, argvBufPtr);

    return __WASI_ESUCCESS;
}

I32 s__gettid()
{
    faabric::util::getLogger()->debug("S - gettid");
    return FAKE_TID;
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env, "geteuid", I32, geteuid)
{
    faabric::util::getLogger()->debug("S - geteuid");
    return FAKE_UID;
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env, "getegid", I32, getegid)
{
    faabric::util::getLogger()->debug("S - getegid");
    return FAKE_GID;
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env, "getgrgid", I32, getgrgid, I32 a)
{
    faabric::util::getLogger()->debug("S - getgrgid {}", a);
    return FAKE_GID;
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env, "getgrnam", I32, getgrnam, I32 a)
{
    faabric::util::getLogger()->debug("S - getgrnam {}", a);
    return 0;
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env, "setgrent", void, setgrent)
{
    faabric::util::getLogger()->debug("S - setgrent");
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env, "getgrent", I32, getgrent)
{
    faabric::util::getLogger()->debug("S - getgrent");
    return 0;
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env, "endgrent", void, endgrent)
{
    faabric::util::getLogger()->debug("S - endgrent");
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env, "getpwuid", I32, getpwuid, I32 uid)
{
    faabric::util::getLogger()->debug("S - getpwuid {}", uid);

    if (uid != FAKE_UID) {
        throw std::runtime_error("Attempting to get pwd for non fake UID");
    }

    // Set up strings to place in passwd
    std::string fakeName = std::string(FAKE_NAME);
    std::string fakeDir = std::string(FAKE_WORKING_DIR);

    // Create enough memory
    size_t nameOffset = sizeof(wasm_passwd);
    size_t dirOffset = nameOffset + fakeName.size() + 1;
    size_t newMemSize = dirOffset + fakeDir.size();

    size_t pageAlignedSize = roundUpToWasmPageAligned(newMemSize);
    U32 wasmMemPtr = getExecutingWAVMModule()->growMemory(pageAlignedSize);

    // Work out the pointers to the strings in wasm memory
    U32 namePtr = wasmMemPtr + nameOffset;
    U32 dirPtr = wasmMemPtr + dirOffset;

    // Get reference to the struct in wasm memory
    auto wasmPasswd = &Runtime::memoryRef<wasm_passwd>(
      getExecutingWAVMModule()->defaultMemory, wasmMemPtr);
    wasmPasswd->pw_uid = FAKE_UID;
    wasmPasswd->pw_gid = FAKE_GID;
    wasmPasswd->pw_dir = dirPtr;
    wasmPasswd->pw_name = namePtr;

    // Copy the strings into place
    std::copy(fakeName.begin(),
              fakeName.end(),
              reinterpret_cast<char*>(wasmPasswd) + nameOffset);
    std::copy(fakeDir.begin(),
              fakeDir.end(),
              reinterpret_cast<char*>(wasmPasswd) + dirOffset);

    return wasmMemPtr;
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env, "getuid", I32, getuid)
{
    faabric::util::getLogger()->debug("S - getuid");
    return FAKE_UID;
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env, "getgid", I32, getgid)
{
    faabric::util::getLogger()->debug("S - getgid");
    return FAKE_GID;
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env, "getpid", I32, getpid)
{
    faabric::util::getLogger()->debug("S - getpid");
    return FAKE_PID;
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env, "getppid", I32, getppid)
{
    throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
}

I32 s__exit(I32 a, I32 b)
{
    faabric::util::getLogger()->debug("S - exit - {} {}", a, b);
    throw(WasmExitException(a));
}

WAVM_DEFINE_INTRINSIC_FUNCTION(wasi,
                               "proc_exit",
                               void,
                               wasi_proc_exit,
                               I32 retCode)
{
    faabric::util::getLogger()->debug("S - proc_exit - {}", retCode);
    throw(WasmExitException(retCode));
}

I32 s__sched_getaffinity(I32 pid, I32 cpuSetSize, I32 maskPtr)
{
    const std::shared_ptr<spdlog::logger>& logger = faabric::util::getLogger();
    logger->debug("S - sched_getaffinity - {} {} {}", pid, cpuSetSize, maskPtr);

    // Native pointer to buffer
    Runtime::Memory* memoryPtr = getExecutingWAVMModule()->defaultMemory;
    U8* hostBufPtr = &Runtime::memoryRef<U8>(memoryPtr, (Uptr)maskPtr);

    // Fill in a mask for the required number of processors
    std::fill(hostBufPtr, hostBufPtr + FAKE_N_PROCESSORS, 1);

    return 0;
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "confstr",
                               I32,
                               confstr,
                               I32 a,
                               I32 b,
                               I32 c)
{
    faabric::util::getLogger()->debug("S - confstr - {} {} {}", a, b, c);

    // Return zero as if no confstr variables have a value set
    return 0;
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env, "abort", void, abort)
{
    faabric::util::getLogger()->debug("S - abort");
    throw(WasmExitException(0));
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env, "exit", void, exit, I32 a)
{
    faabric::util::getLogger()->debug("S - exit - {}", a);
    throw(WasmExitException(a));
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env, "_Exit", void, _Exit, I32 a)
{
    faabric::util::getLogger()->debug("S - _Exit - {}", a);
    throw(WasmExitException(a));
}

/**
 * Allowing straight-through access to sysconf my not be wise. Should revisit
 * this.
 */
WAVM_DEFINE_INTRINSIC_FUNCTION(env, "sysconf", I32, _sysconf, I32 a)
{
    const std::shared_ptr<spdlog::logger>& logger = faabric::util::getLogger();
    logger->debug("S - _sysconf - {}", a);

    if (a == _SC_NPROCESSORS_ONLN) {
        return sysconf(a);
    } else {
        logger->error("Called sysconf with unsupported param - {}", a);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env, "uname", I32, uname, I32 bufPtr)
{
    faabric::util::getLogger()->debug("S - uname - {}", bufPtr);

    // Native pointer to buffer
    Runtime::Memory* memoryPtr = getExecutingWAVMModule()->defaultMemory;
    U8* hostBufPtr = &Runtime::memoryRef<U8>(memoryPtr, (Uptr)bufPtr);

    // Fake system info
    // TODO - should probably give some valid stuff here in case we break
    // something
    wasm_utsname s{ .sysname = "Linux",
                    .nodename = "faasm",
                    .release = "1.0.0",
                    .version = "Faasm 123",
                    .machine = "x86", // Probably safest in 32-bit wasm env
                    .domainname = "(none)" };

    // Copy fake info into place
    size_t structSize = sizeof(wasm_utsname);
    auto structPtr = BYTES(&s);
    std::copy(structPtr, structPtr + structSize, hostBufPtr);

    return 0;
}

WAVM_DEFINE_INTRINSIC_FUNCTION(wasi,
                               "environ_sizes_get",
                               I32,
                               wasi_environ_sizes_get,
                               I32 environCountPtr,
                               I32 environBuffSizePtr)
{
    faabric::util::getLogger()->debug(
      "S - environ_sizes_get - {} {}", environCountPtr, environBuffSizePtr);

    WAVMWasmModule* module = getExecutingWAVMModule();
    WasmEnvironment& wasmEnv = module->getWasmEnvironment();

    Runtime::memoryRef<U32>(module->defaultMemory, environCountPtr) =
      wasmEnv.getEnvCount();
    Runtime::memoryRef<U32>(module->defaultMemory, environBuffSizePtr) =
      wasmEnv.getEnvBufferSize();

    return __WASI_ESUCCESS;
}

WAVM_DEFINE_INTRINSIC_FUNCTION(wasi,
                               "environ_get",
                               I32,
                               wasi_environ_get,
                               I32 environPtrs,
                               I32 environBuf)
{
    faabric::util::getLogger()->debug(
      "S - environ_get - {} {}", environPtrs, environBuf);

    WAVMWasmModule* module = getExecutingWAVMModule();
    module->writeWasmEnvToMemory(environPtrs, environBuf);

    return __WASI_ESUCCESS;
}

// Random
I32 s__getrandom(I32 bufPtr, I32 bufLen, I32 flags)
{
    faabric::util::getLogger()->debug(
      "S - getrandom - {} {} {}", bufPtr, bufLen, flags);

    auto hostBuf = &Runtime::memoryRef<U8>(
      getExecutingWAVMModule()->defaultMemory, (Uptr)bufPtr);

    return getrandom(hostBuf, bufLen, flags);
}

WAVM_DEFINE_INTRINSIC_FUNCTION(wasi,
                               "random_get",
                               I32,
                               wasi_random_get,
                               I32 bufPtr,
                               I32 bufLen)
{
    faabric::util::getLogger()->debug("S - random_get - {} {}", bufPtr, bufLen);

    auto hostBuf = &Runtime::memoryRef<U8>(
      getExecutingWAVMModule()->defaultMemory, (Uptr)bufPtr);

    getrandom(hostBuf, bufLen, 0);

    return __WASI_ESUCCESS;
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "getcwd",
                               I32,
                               getcwd,
                               I32 bufPtr,
                               I32 bufLen)
{
    faabric::util::getLogger()->debug("S - getcwd - {} {}", bufPtr, bufLen);

    Runtime::Memory* memoryPtr = getExecutingWAVMModule()->defaultMemory;
    char* hostBuf =
      Runtime::memoryArrayPtr<char>(memoryPtr, (Uptr)bufPtr, (Uptr)bufLen);

    // Fake working directory
    std::strcpy(hostBuf, FAKE_WORKING_DIR);

    // Note, this needs to return the buffer on success, NOT zero
    return bufPtr;
}

// --------------------------
// Unsupported
// --------------------------

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "__h_errno_location",
                               I32,
                               __h_errno_location)
{
    throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env, "ttyname", I32, ttyname, I32 a)
{
    throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env, "getpwnam", I32, getpwnam, I32 a)
{
    throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "getresuid",
                               I32,
                               getresuid,
                               I32 a,
                               I32 b,
                               I32 c)
{
    faabric::util::getLogger()->debug("S - getresuid - {} {} {}", a, b, c);
    throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "getresgid",
                               I32,
                               getresgid,
                               I32 a,
                               I32 b,
                               I32 c)
{
    faabric::util::getLogger()->debug("S - getresgid - {} {} {}", a, b, c);
    throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env, "getrusage", I32, getrusage, I32 a, I32 b)
{
    faabric::util::getLogger()->debug("S - getrusage - {} {}", a, b);
    throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env, "getrlimit", I32, getrlimit, I32 a, I32 b)
{
    faabric::util::getLogger()->debug("S - getrlimit - {} {}", a, b);
    throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env, "setrlimit", I32, setrlimit, I32 a, I32 b)
{
    faabric::util::getLogger()->debug("S - setrlimit - {} {}", a, b);
    throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env, "longjmp", void, longjmp, I32 a, U32 b)
{
    faabric::util::getLogger()->debug("S - longjmp - {} {}", a, b);
    throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env, "setjmp", I32, setjmp, I32 a)
{
    faabric::util::getLogger()->debug("S - setjmp - {}", a);
    throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "__errno_location",
                               I32,
                               wasi__errno_location)
{
    throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
}

void envLink() {}
}
