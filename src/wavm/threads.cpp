#include "WAVMWasmModule.h"
#include "faabric/util/logging.h"
#include "syscalls.h"

#include <linux/futex.h>

#include <faabric/util/config.h>
#include <wasm/chaining.h>

#include <WAVM/Platform/Thread.h>
#include <WAVM/Runtime/Intrinsics.h>
#include <WAVM/Runtime/Runtime.h>

using namespace WAVM;

namespace wasm {
struct PThreadArgs
{
    WAVMWasmModule* parentModule;
    faabric::Message* parentCall;
    WasmThreadSpec* spec;
};

// Map of tid to pointer to local thread
static thread_local std::unordered_map<I32, WAVM::Platform::Thread*>
  localThreads;

// Map of tid to message ID for chained calls
static thread_local std::unordered_map<I32, unsigned int> chainedThreads;

// Flag to say whether we've spawned a thread
static std::string activeSnapshotKey;
static size_t threadSnapshotSize;

// Map of file mutexes

// ---------------------------------------------
// LIBC THREADING
// ---------------------------------------------

/*
 * Locks the given file pointer and returns 1 if an unlock is needed, otherwise
 * zero.
 *
 * Must also be reentrant.
 *
 * See musl implementation.
 * https://github.com/faasm/wasi-libc/blob/faasm/libc-top-half/musl/src/stdio/__lockfile.c
 */
WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "__lockfile",
                               I32,
                               __lockfile,
                               I32 filePtr)
{
    faabric::util::getLogger()->debug("S - __lockfile {}", filePtr);

    return 1;
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "__unlockfile",
                               void,
                               __unlockfile,
                               I32 filePtr)
{
    faabric::util::getLogger()->debug("S - __unlockfile {}", filePtr);
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "__lock",
                               void,
                               __lock,
                               I32 ptr)
{
    faabric::util::getLogger()->debug("S - __lock {}", ptr);
}

// ---------------------------------------------
// PTHREADS
// ---------------------------------------------

I64 createPthread(void* threadSpecPtr)
{
    auto pArg = reinterpret_cast<PThreadArgs*>(threadSpecPtr);

    setExecutingModule(pArg->parentModule);
    setExecutingCall(pArg->parentCall);

    I64 res = getExecutingWAVMModule()->executeThreadLocally(*pArg->spec);

    // Delete the spec, no longer needed
    delete[] pArg->spec->funcArgs;
    delete pArg->spec;
    delete pArg;

    return res;
}

/**
 * We intercept the pthread API at a high level, hence we control the whole
 * lifecycle. For this reason, we mostly ignore the contents of the pthread
 * struct and pthread attrs.
 *
 * We just use the int value of the pthread pointer to act as its ID (to be
 * passed around the different pthread functions).
 *
 * In the "chain" threading mode, we spawn threads as chained function calls,
 * which may get executed on another host. To enable this we create a zygote
 * from which these "thread" calls can be spawned on another host.
 *
 * @param pthreadPtr - pointer to the pthread struct
 * @param attrPtr - pointer to the pthread attr struct
 * @param entryFunc - function table index for the entrypoint
 * @param argsPtr - args pointer for the function
 */
WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "pthread_create",
                               I32,
                               pthread_create,
                               I32 pthreadPtr,
                               I32 attrPtr,
                               I32 entryFunc,
                               I32 argsPtr)
{
    const std::shared_ptr<spdlog::logger>& logger = faabric::util::getLogger();
    logger->debug("S - pthread_create - {} {} {} {}",
                  pthreadPtr,
                  attrPtr,
                  entryFunc,
                  argsPtr);

    // Set the bits we care about on the pthread struct
    // NOTE - setting the initial pointer is crucial for inter-operation with
    // existing C code
    WAVMWasmModule* thisModule = getExecutingWAVMModule();
    wasm_pthread* pthreadHost =
      &Runtime::memoryRef<wasm_pthread>(thisModule->defaultMemory, pthreadPtr);
    pthreadHost->selfPtr = pthreadPtr;

    faabric::util::SystemConfig& conf = faabric::util::getSystemConfig();
    if (conf.threadMode == "local") {
        // Spawn a local thread
        Runtime::Object* funcObj =
          Runtime::getTableElement(thisModule->defaultTable, entryFunc);
        Runtime::Function* func = Runtime::asFunction(funcObj);

        // Note that the spec needs to outlast the scope of this function, so
        // nothing can be created on the stack (this is deleted once the thread
        // is finished)
        auto threadArgs = new IR::UntaggedValue[1];
        threadArgs[0] = argsPtr;

        auto spec = new WasmThreadSpec();
        spec->contextRuntimeData = contextRuntimeData;
        spec->func = func;
        spec->funcArgs = threadArgs;
        spec->stackTop = thisModule->allocateThreadStack();

        auto pArgs = new PThreadArgs();
        pArgs->parentModule = thisModule;
        pArgs->parentCall = getExecutingCall();
        pArgs->spec = spec;

        // Spawn the thread
        localThreads.insert(
          { pthreadPtr, Platform::createThread(0, createPthread, pArgs) });

    } else if (conf.threadMode == "chain") {
        // Create a new zygote if one isn't already active
        if (activeSnapshotKey.empty()) {
            int callId = getExecutingCall()->id();
            activeSnapshotKey =
              std::string("pthread_snapshot_") + std::to_string(callId);
            threadSnapshotSize = thisModule->snapshotToState(activeSnapshotKey);
        }

        // Chain the threaded call
        int chainedCallId = spawnChainedThread(
          activeSnapshotKey, threadSnapshotSize, entryFunc, argsPtr);

        // Record this thread -> call ID
        chainedThreads.insert({ pthreadPtr, chainedCallId });
    } else {
        logger->error("Unsupported threading mode: {}", conf.threadMode);
        throw std::runtime_error("Unsupported threading mode");
    }

    return 0;
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "pthread_join",
                               I32,
                               pthread_join,
                               I32 pthreadPtr,
                               I32 resPtrPtr)
{
    const std::shared_ptr<spdlog::logger>& logger = faabric::util::getLogger();
    logger->debug("S - pthread_join - {} {}", pthreadPtr, resPtrPtr);

    faabric::util::SystemConfig& conf = faabric::util::getSystemConfig();
    int returnValue;
    if (conf.threadMode == "local") {
        // Get the local thread and remove it from the local map
        Platform::Thread* thread = localThreads[pthreadPtr];
        localThreads.erase(pthreadPtr);

        // Join it
        returnValue = Platform::joinThread(thread);
    } else if (conf.threadMode == "chain") {
        // Await the remotely chained thread
        unsigned int callId = chainedThreads[pthreadPtr];
        returnValue = awaitChainedCall(callId);

        // Remove record for the remote thread
        chainedThreads.erase(pthreadPtr);

        // If this is the last active thread, reset the zygote key
        if (chainedThreads.empty()) {
            activeSnapshotKey = "";
        }
    } else {
        logger->error("Unsupported threading mode: {}", conf.threadMode);
        throw std::runtime_error("Unsupported threading mode");
    }

    // This function is passed a pointer to a pointer for the result,
    // so we dereference it once and are writing an integer (i.e. a wasm
    // pointer)
    auto resPtr = &Runtime::memoryRef<I32>(
      getExecutingWAVMModule()->defaultMemory, resPtrPtr);
    *resPtr = returnValue;

    return 0;
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "pthread_exit",
                               void,
                               pthread_exit,
                               I32 code)
{
    faabric::util::getLogger()->debug("S - pthread_exit - {}", code);
}

// ----------------------------------------------
// FUTEX
// ----------------------------------------------

I32 s__futex(I32 uaddrPtr,
             I32 futex_op,
             I32 val,
             I32 timeoutPtr,
             I32 uaddr2Ptr,
             I32 other)
{
    const std::shared_ptr<spdlog::logger>& logger = faabric::util::getLogger();
    std::string opStr;
    int returnValue = 0;

    // Reference to uaddr
    // The value pointed to by uaddr is always a four byte integer
    Runtime::Memory* memoryPtr = getExecutingWAVMModule()->defaultMemory;
    I32* actualValPtr = &Runtime::memoryRef<I32>(memoryPtr, (Uptr)uaddrPtr);
    I32 actualVal = *actualValPtr;

    if (futex_op == FUTEX_WAIT || futex_op == FUTEX_WAIT_PRIVATE) {
        // Waiting needs to be handled using Faasm mechanisms so we ignore
        opStr = futex_op == FUTEX_WAIT ? "FUTEX_WAIT" : "FUTEX_WAIT_PRIVATE";

        // Need to set the value to be unequal to the expected, otherwise we get
        // into a loop
        I32 newVal = actualVal + 1;
        std::copy(&newVal, &newVal + sizeof(I32), actualValPtr);

        // val here is the expected value, we need to make sure the address
        // still has that value
        returnValue = 0;
    } else if (futex_op == FUTEX_WAKE || futex_op == FUTEX_WAKE_PRIVATE) {
        // Waking also needs to be handled with Faasm mechanisms so we also
        // ignore
        opStr = futex_op == FUTEX_WAKE ? "FUTEX_WAKE" : "FUTEX_WAKE_PRIVATE";

        // val here means "max waiters to wake"
        returnValue = val;
    } else {
        logger->error("Unsupported futex syscall with operation {}", futex_op);
        throw std::runtime_error("Unuspported futex syscall");
    }

    logger->debug("S - futex - {} {} {} {} {} {} (uaddr = {})",
                  uaddrPtr,
                  opStr,
                  val,
                  timeoutPtr,
                  uaddr2Ptr,
                  other,
                  actualVal);
    return returnValue;
}

/*
 * --------------------------
 * STUBBED PTHREADS
 * --------------------------
 */

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "pthread_mutex_init",
                               I32,
                               pthread_mutex_init,
                               I32 a,
                               I32 b)
{
    // faabric::util::getLogger()->trace("S - pthread_mutex_init {} {}", a, b);

    return 0;
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "pthread_cond_init",
                               I32,
                               pthread_cond_init,
                               I32 a,
                               I32 b)
{
    // faabric::util::getLogger()->trace("S - pthread_cond_init {} {}", a, b);

    return 0;
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "pthread_mutex_lock",
                               I32,
                               pthread_mutex_lock,
                               I32 a)
{
    // faabric::util::getLogger()->trace("S - pthread_mutex_lock {}", a);

    return 0;
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "pthread_cond_signal",
                               I32,
                               pthread_cond_signal,
                               I32 a)
{
    // faabric::util::getLogger()->trace("S - pthread_cond_signal {}", a);

    return 0;
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "pthread_mutex_unlock",
                               I32,
                               pthread_mutex_unlock,
                               I32 a)
{
    // faabric::util::getLogger()->trace("S - pthread_mutex_unlock {}", a);

    return 0;
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "pthread_mutex_destroy",
                               I32,
                               pthread_mutex_destroy,
                               I32 a)
{
    // faabric::util::getLogger()->trace("S - pthread_mutex_destroy {}", a);

    return 0;
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env, "pthread_self", I32, pthread_self)
{
    // faabric::util::getLogger()->trace("S - pthread_self");

    return 0;
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "pthread_key_create",
                               I32,
                               s__pthread_key_create,
                               I32 a,
                               I32 b)
{
    // faabric::util::getLogger()->trace("S - pthread_key_create {} {}", a, b);

    return 0;
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "pthread_key_delete",
                               I32,
                               s__pthread_key_delete,
                               I32 a)
{
    // faabric::util::getLogger()->trace("S - pthread_key_delete {}", a);

    return 0;
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "pthread_getspecific",
                               I32,
                               s__pthread_getspecific,
                               I32 a)
{
    // faabric::util::getLogger()->trace("S - pthread_getspecific {}", a);

    return 0;
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "pthread_setspecific",
                               I32,
                               s__pthread_setspecific,
                               I32 a,
                               I32 b)
{
    // faabric::util::getLogger()->trace("S - pthread_setspecific {} {}", a, b);

    return 0;
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "pthread_mutex_trylock",
                               I32,
                               s__pthread_mutex_trylock,
                               I32 a)
{
    // faabric::util::getLogger()->trace("S - pthread_mutex_trylock {}", a);

    return 0;
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "pthread_cond_destroy",
                               I32,
                               pthread_cond_destroy,
                               I32 a)
{
    // faabric::util::getLogger()->trace("S - pthread_cond_destroy {}", a);

    return 0;
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "pthread_cond_broadcast",
                               I32,
                               pthread_cond_broadcast,
                               I32 a)
{
    // faabric::util::getLogger()->trace("S - pthread_cond_broadcast {}", a);

    return 0;
}

/*
 * --------------------------
 * Unsupported
 * --------------------------
 */

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "pthread_equal",
                               I32,
                               pthread_equal,
                               I32 a,
                               I32 b)
{
    faabric::util::getLogger()->trace("S - pthread_equal {} {}", a, b);
    throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "pthread_cond_timedwait",
                               I32,
                               pthread_cond_timedwait,
                               I32 a,
                               I32 b,
                               I32 c)
{
    throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "pthread_cond_wait",
                               I32,
                               pthread_cond_wait,
                               I32 a,
                               I32 b)
{
    throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "pthread_attr_init",
                               I32,
                               s__pthread_attr_init,
                               I32 a)
{
    throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "pthread_attr_setstacksize",
                               I32,
                               s__pthread_attr_setstacksize,
                               I32 a,
                               I32 b)
{
    throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "pthread_attr_destroy",
                               I32,
                               s__pthread_attr_destroy,
                               I32 a)
{
    throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "pthread_detach",
                               I32,
                               s__pthread_detach,
                               I32 a)
{
    throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
}

void threadsLink() {}
}
