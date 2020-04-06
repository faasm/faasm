#include "WasmModule.h"
#include "syscalls.h"

#include <linux/futex.h>

#include <util/config.h>

#include <WAVM/Runtime/Runtime.h>
#include <WAVM/Runtime/Intrinsics.h>
#include <WAVM/Platform/Thread.h>


#define PTHREAD_STACK_SIZE (2 * ONE_MB_BYTES)

namespace wasm {
    // Map of tid to pointer to local thread
    static thread_local std::unordered_map<I32, WAVM::Platform::Thread *> localThreads;

    // Map of tid to message ID for chained calls
    static thread_local std::unordered_map<I32, unsigned int> chainedThreads;

    // Flag to say whether we've spawned a thread
    static std::string activeSnapshotKey;
    static size_t threadSnapshotSize;

    I32 s__fork() {
        util::getLogger()->debug("S - fork");
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    // ---------------------------------------
    // Signals
    //
    // The runtime itself will handle thread creation/ deletion etc.
    // So we don't worry about signal-related calls
    // ---------------------------------------

    I32 s__rt_sigprocmask(I32 how, I32 sigSetPtr, I32 oldSetPtr, I32 sigsetsize) {
        util::getLogger()->debug("S - rt_sigprocmask - {} {} {} {}", how, sigSetPtr, oldSetPtr, sigsetsize);

        return 0;
    }

    // ---------------------------------------
    // pthreads
    //
    // We stub out a lot of the standard guts of pthread to intercept at
    // the highest level (i.e. pthread_create, pthread_join etc.)
    // ---------------------------------------

    I64 createPthread(void *threadSpecPtr) {
        auto threadSpec = reinterpret_cast<WasmThreadSpec *>(threadSpecPtr);
        I64 res = getExecutingModule()->executeThread(*threadSpec);

        // Delete the spec, no longer needed
        delete[] threadSpec->funcArgs;
        delete threadSpec;

        return res;
    }

    /**
     * We intercept the pthread API at a high level, hence we control the whole
     * lifecycle. For this reason, we mostly ignore the contents of the pthread struct
     * and pthread attrs.
     *
     * We just use the int value of the pthread pointer to act as its ID (to be passed
     * around the different pthread functions).
     *
     * In the "chain" threading mode, we spawn threads as chained function calls, which
     * may get executed on another host. To enable this we create a zygote from which
     * these "thread" calls can be spawned on another host.
     *
     * @param pthreadPtr - pointer to the pthread struct
     * @param attrPtr - pointer to the pthread attr struct
     * @param entryFunc - function table index for the entrypoint
     * @param argsPtr - args pointer for the function
     */
    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "pthread_create", I32, pthread_create, I32 pthreadPtr, I32 attrPtr,
                                   I32 entryFunc, I32 argsPtr) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->debug("S - pthread_create - {} {} {} {}", pthreadPtr, attrPtr, entryFunc, argsPtr);

        // Set the bits we care about on the pthread struct
        // NOTE - setting the initial pointer is crucial for inter-operation with existing C code
        WasmModule *thisModule = getExecutingModule();
        wasm_pthread *pthreadHost = &Runtime::memoryRef<wasm_pthread>(thisModule->defaultMemory, pthreadPtr);
        pthreadHost->selfPtr = pthreadPtr;

        util::SystemConfig &conf = util::getSystemConfig();
        if (conf.threadMode == "local") {
            // Spawn a local thread
            Runtime::Object *funcObj = Runtime::getTableElement(thisModule->defaultTable, entryFunc);
            Runtime::Function *func = Runtime::asFunction(funcObj);

            // Note that the spec needs to outlast the scope of this function, so
            // nothing can be created on the stack (this is deleted once the thread is finished)
            auto threadArgs = new IR::UntaggedValue[1];
            threadArgs[0] = argsPtr;

            auto spec = new WasmThreadSpec();
            spec->contextRuntimeData = contextRuntimeData;
            spec->parentModule = thisModule;
            spec->parentCall = getExecutingCall();
            spec->func = func;
            spec->funcArgs = threadArgs;
            spec->stackSize = PTHREAD_STACK_SIZE;

            // Spawn the thread
            localThreads.insert({pthreadPtr, Platform::createThread(0, createPthread, spec)});

        } else if (conf.threadMode == "chain") {
            // Create a new zygote if one isn't already active
            if (activeSnapshotKey.empty()) {
                int callId = getExecutingCall()->id();
                activeSnapshotKey = std::string("pthread_snapshot_") + std::to_string(callId);
                threadSnapshotSize = thisModule->snapshotToState(activeSnapshotKey);
            }

            // Chain the threaded call
            int chainedCallId = spawnChainedThread(activeSnapshotKey, threadSnapshotSize, entryFunc, argsPtr);

            // Record this thread -> call ID
            chainedThreads.insert({pthreadPtr, chainedCallId});
        } else {
            logger->error("Unsupported threading mode: {}", conf.threadMode);
            throw std::runtime_error("Unsupported threading mode");
        }
        return 0;
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "pthread_join", I32, pthread_join, I32 pthreadPtr, I32 resPtrPtr) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->debug("S - pthread_join - {} {}", pthreadPtr, resPtrPtr);

        util::SystemConfig &conf = util::getSystemConfig();
        int returnValue;
        if (conf.threadMode == "local") {
            // Get the local thread and remove it from the local map
            Platform::Thread *thread = localThreads[pthreadPtr];
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
        // so we dereference it once and are writing an integer (i.e. a wasm pointer)
        auto resPtr = &Runtime::memoryRef<I32>(getExecutingModule()->defaultMemory, resPtrPtr);
        *resPtr = returnValue;

        return 0;
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "pthread_exit", void, pthread_exit, I32 code) {
        util::getLogger()->debug("S - pthread_exit - {}", code);

    }

    I32 s__futex(I32 uaddrPtr, I32 futex_op, I32 val, I32 timeoutPtr, I32 uaddr2Ptr, I32 other) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        std::string opStr;
        int returnValue = 0;

        // Reference to uaddr
        // The value pointed to by uaddr is always a four byte integer
        Runtime::Memory *memoryPtr = getExecutingModule()->defaultMemory;
        I32 *actualValPtr = &Runtime::memoryRef<I32>(memoryPtr, (Uptr) uaddrPtr);
        I32 actualVal = *actualValPtr;

        if (futex_op == FUTEX_WAIT || futex_op == FUTEX_WAIT_PRIVATE) {
            // Waiting needs to be handled using Faasm mechanisms so we ignore
            opStr = futex_op == FUTEX_WAIT ? "FUTEX_WAIT" : "FUTEX_WAIT_PRIVATE";

            // Need to set the value to be unequal to the expected, otherwise we get into a loop
            I32 newVal = actualVal + 1;
            std::copy(&newVal, &newVal + sizeof(I32), actualValPtr);

            // val here is the expected value, we need to make sure the address still has that value
            returnValue = 0;
        } else if (futex_op == FUTEX_WAKE || futex_op == FUTEX_WAKE_PRIVATE) {
            // Waking also needs to be handled with Faasm mechanisms so we also ignore
            opStr = futex_op == FUTEX_WAKE ? "FUTEX_WAKE" : "FUTEX_WAKE_PRIVATE";

            // val here means "max waiters to wake"
            returnValue = val;
        } else {
            logger->error("Unsupported futex syscall with operation {}", futex_op);
            throw std::runtime_error("Unuspported futex syscall");
        }

        logger->debug("S - futex - {} {} {} {} {} {} (uaddr = {})", uaddrPtr, opStr, val, timeoutPtr, uaddr2Ptr, other,
                      actualVal);
        return returnValue;
    }
}
