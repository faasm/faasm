#include "syscalls.h"

#include <faabric/proto/faabric.pb.h>
#include <faabric/scheduler/Scheduler.h>
#include <faabric/snapshot/SnapshotRegistry.h>
#include <faabric/util/config.h>
#include <faabric/util/func.h>
#include <faabric/util/logging.h>
#include <threads/MutexManager.h>
#include <wasm/WasmModule.h>
#include <wasm/chaining.h>
#include <wavm/WAVMWasmModule.h>

#include <linux/futex.h>

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

// Record of whether this thread has already got a snapshot being used to spawn
// other threads.
static thread_local std::string currentSnapshotKey;

// ---------------------------------------------
// PTHREADS
// ---------------------------------------------

I64 createPthread(void* threadSpecPtr)
{
    auto pArg = reinterpret_cast<PThreadArgs*>(threadSpecPtr);

    setExecutingModule(pArg->parentModule);
    setExecutingCall(pArg->parentCall);

    I64 res = getExecutingWAVMModule()->executeThreadLocally(*pArg->spec);

    // Notify the scheduler that we've completed executing the thread
    faabric::scheduler::Scheduler& sch = faabric::scheduler::getScheduler();
    sch.notifyCallFinished(*pArg->parentCall);

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

    faabric::Message* originalCall = getExecutingCall();
    std::string funcStr = faabric::util::funcToString(*originalCall, true);
    faabric::scheduler::Scheduler& sch = faabric::scheduler::getScheduler();

    // Set the bits we care about on the pthread struct
    // NOTE - setting the initial pointer is crucial for inter-operation with
    // existing C code
    WAVMWasmModule* thisModule = getExecutingWAVMModule();
    wasm_pthread* pthreadNative =
      &Runtime::memoryRef<wasm_pthread>(thisModule->defaultMemory, pthreadPtr);
    pthreadNative->selfPtr = pthreadPtr;

    // Create a new snapshot if one isn't already active
    if (currentSnapshotKey.empty()) {
        currentSnapshotKey = thisModule->snapshot(false);

        logger->debug(
          "Setting pthread snapshot for {} ({})", funcStr, currentSnapshotKey);
    }

    faabric::Message threadCall = faabric::util::messageFactory(
      originalCall->user(), originalCall->function());
    threadCall.set_isasync(true);

    // Snapshot details
    threadCall.set_snapshotkey(currentSnapshotKey);

    // Function pointer and args
    // NOTE - with a pthread interface we only ever pass the function a single
    // pointer argument, hence we use the input data here to hold this argument
    // as a string
    threadCall.set_funcptr(entryFunc);
    threadCall.set_inputdata(std::to_string(argsPtr));

    // Set up the request
    std::vector<faabric::Message> msgs = { threadCall };
    faabric::BatchExecuteRequest req = faabric::util::batchExecFactory(msgs);
    req.set_type(faabric::BatchExecuteRequest::THREADS);

    // Submit it
    std::vector<std::string> hosts = sch.callFunctions(req);

    // Check if the scheduler expects us to execute locally
    bool execLocal = hosts.at(0) == "";

    if (execLocal) {
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

        auto pArgs = new PThreadArgs();
        pArgs->parentModule = thisModule;
        pArgs->parentCall = getExecutingCall();
        pArgs->spec = spec;

        // Spawn the thread
        localThreads.insert(
          { pthreadPtr, Platform::createThread(0, createPthread, pArgs) });

    } else {
        // Record this thread -> call ID
        chainedThreads.insert({ pthreadPtr, threadCall.id() });
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

    int returnValue;

    if (localThreads.count(pthreadPtr) > 0) {
        // Get the local thread and remove it from the local map
        Platform::Thread* thread = localThreads[pthreadPtr];
        localThreads.erase(pthreadPtr);

        // Join it
        logger->debug("Awaiting local pthread: {}", pthreadPtr);
        returnValue = Platform::joinThread(thread);
    } else {
        // Await the remotely chained thread
        unsigned int callId = chainedThreads[pthreadPtr];
        logger->debug("Awaiting remote pthread: {} ({})", pthreadPtr, callId);
        returnValue = awaitChainedCall(callId);

        // Remove record for the remote thread
        chainedThreads.erase(pthreadPtr);
    }

    // If we're done with executing threads, remove the snapshot
    if (chainedThreads.empty() && localThreads.empty()) {
        logger->debug("Finished with snapshot: {}", currentSnapshotKey);
        currentSnapshotKey = "";
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

// --------------------------
// PTHREAD MUTEXES - We support pthread mutexes locally as they're important to
// support thread-safe libc operations
// --------------------------

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "pthread_mutex_init",
                               I32,
                               pthread_mutex_init,
                               I32 mx,
                               I32 attr)
{
    faabric::util::getLogger()->debug("S - pthread_mutex_init {} {}", mx, attr);
    threads::getMutexManager().createMutex(mx);
    return 0;
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "pthread_mutex_lock",
                               I32,
                               pthread_mutex_lock,
                               I32 mx)
{
    faabric::util::getLogger()->debug("S - pthread_mutex_lock {}", mx);
    threads::getMutexManager().lockMutex(mx);
    return 0;
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "pthread_mutex_trylock",
                               I32,
                               s__pthread_mutex_trylock,
                               I32 mx)
{
    faabric::util::getLogger()->debug("S - pthread_mutex_trylock {}", mx);
    bool success = threads::getMutexManager().tryLockMutex(mx);
    if (success) {
        return 0;
    } else {
        return EBUSY;
    }
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "pthread_mutex_unlock",
                               I32,
                               pthread_mutex_unlock,
                               I32 mx)
{
    faabric::util::getLogger()->debug("S - pthread_mutex_unlock {}", mx);
    threads::getMutexManager().unlockMutex(mx);
    return 0;
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "pthread_mutex_destroy",
                               I32,
                               pthread_mutex_destroy,
                               I32 mx)
{
    faabric::util::getLogger()->debug("S - pthread_mutex_destroy {}", mx);
    threads::getMutexManager().destroyMutex(mx);
    return 0;
}

// --------------------------
// STUBBED PTHREADS - We can safely ignore the following functions
// --------------------------

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "pthread_mutexattr_init",
                               I32,
                               pthread_mutexattr_init,
                               I32 a)
{
    faabric::util::getLogger()->debug("S - pthread_mutexattr_init {}", a);

    return 0;
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "pthread_mutexattr_destroy",
                               I32,
                               pthread_mutexattr_destroy,
                               I32 a)
{
    faabric::util::getLogger()->debug("S - pthread_mutexattr_destroy {}", a);

    return 0;
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "pthread_cond_init",
                               I32,
                               pthread_cond_init,
                               I32 a,
                               I32 b)
{
    faabric::util::getLogger()->debug("S - pthread_cond_init {} {}", a, b);

    return 0;
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "pthread_cond_signal",
                               I32,
                               pthread_cond_signal,
                               I32 a)
{
    faabric::util::getLogger()->debug("S - pthread_cond_signal {}", a);

    return 0;
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env, "pthread_self", I32, pthread_self)
{
    faabric::util::getLogger()->debug("S - pthread_self");

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

// --------------------------
// Unsupported
// --------------------------

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
