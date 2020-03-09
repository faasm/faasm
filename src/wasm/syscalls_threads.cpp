#include "WasmModule.h"
#include "syscalls.h"

#include <linux/futex.h>

#include <WAVM/Runtime/Runtime.h>
#include <WAVM/Runtime/Intrinsics.h>
#include <WAVM/Platform/Thread.h>


#define PTHREAD_STACK_SIZE (2 * ONE_MB_BYTES)

namespace wasm {
    // Record of the threads spawned so far.
    static thread_local std::unordered_map<I32, WAVM::Platform::Thread *> activeThreads;

    I32 s__fork() {
        util::getLogger()->debug("S - fork");
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    I64 createPthread(void *threadSpecPtr) {
        auto threadSpec = reinterpret_cast<WasmThreadSpec *>(threadSpecPtr);
        I64 res = getExecutingModule()->executeThread(*threadSpec);

        // Delete the spec, no longer needed
        delete[] threadSpec->funcArgs;
        delete threadSpec;

        return res;
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

    /**
     * We intercept the pthread API at a high level, hence we control the whole
     * lifecycle. For this reason, we mostly ignore the contents of the pthread struct
     * and pthread attrs.
     *
     * We just use the int value of the pthread pointer to act as its ID (to be passed
     * around the different pthread functions).
     *
     * @param pthreadPtr - pointer to the pthread struct
     * @param attrPtr - pointer to the pthread attr struct
     * @param entryFunc - function table index for the entrypoint
     * @param argsPtr - args pointer for the function
     */
    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "pthread_create", I32, pthread_create, I32 pthreadPtr, I32 attrPtr,
                                   I32 entryFunc, I32 argsPtr) {
        util::getLogger()->debug("S - pthread_create - {} {} {} {}", pthreadPtr, attrPtr, entryFunc, argsPtr);

        // Set the bits we care about on the pthread struct
        // NOTE - setting the initial pointer is crucial
        WasmModule *thisModule = getExecutingModule();
        wasm_pthread *pthreadHost = &Runtime::memoryRef<wasm_pthread>(thisModule->defaultMemory, pthreadPtr);
        pthreadHost->selfPtr = pthreadPtr;
        
        // Look up the entry function
        Runtime::Object *funcObj = Runtime::getTableElement(thisModule->defaultTable, entryFunc);
        Runtime::Function *func = Runtime::asFunction(funcObj);

        // Note that the spec needs to outlast the scope of this function, so
        // nothing can be created on the stack
        auto threadArgs = new IR::UntaggedValue[1];
        threadArgs[0] = argsPtr;

        WasmThreadSpec *spec = new WasmThreadSpec();
        spec->contextRuntimeData=contextRuntimeData;
        spec->parentModule = thisModule;
        spec->parentCall = getExecutingCall();
        spec->func=func;
        spec->funcArgs=threadArgs;
        spec->stackSize = PTHREAD_STACK_SIZE;

        activeThreads.insert({pthreadPtr, Platform::createThread(0, createPthread, spec)});

        return 0;
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "pthread_join", I32, pthread_join, I32 pthreadPtr, I32 resPtrPtr) {
        util::getLogger()->debug("S - pthread_join - {} {}", pthreadPtr, resPtrPtr);

        Platform::Thread *thread = activeThreads[pthreadPtr];
        int res = Platform::joinThread(thread);

        if(res != 0) {
            throw std::runtime_error("Joined failed pthread");
        }

        return 0;
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "pthread_exit", void, pthread_exit, I32 code) {
        util::getLogger()->debug("S - pthread_exit - {}", code);

    }

//    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "pthread_cond_destroy", I32, pthread_cond_destroy, I32 a) {
//        util::getLogger()->debug("S - pthread_cond_destroy - {}", a);
//
//        return 0;
//    }
//
//    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "pthread_cond_init", I32, pthread_cond_init, I32 a, I32 b) {
//        util::getLogger()->debug("S - pthread_cond_init - {} {}", a, b);
//
//        return 0;
//    }
//
//    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "pthread_cond_signal", I32, pthread_cond_signal, I32 a) {
//        util::getLogger()->debug("S - pthread_cond_signal - {}", a);
//
//        return 0;
//    }
//
//    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "pthread_getspecific", I32, pthread_getspecific, I32 a) {
//        util::getLogger()->debug("S - pthread_getspecific - {}", a);
//
//        return 0;
//    }
//
//    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "pthread_key_create", I32, pthread_key_create, I32 keyPtr, I32 destructor) {
//        util::getLogger()->debug("S - pthread_key_create - {} {}", keyPtr, destructor);
//
//        return 0;
//    }
//
//    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "pthread_key_delete", I32, pthread_key_delete, I32 a) {
//        util::getLogger()->debug("S - pthread_key_delete - {}", a);
//
//        return 0;
//    }
//
//    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "pthread_mutex_destroy", I32, pthread_mutex_destroy, I32 a) {
//        util::getLogger()->debug("S - pthread_mutex_destroy - {}", a);
//
//        return 0;
//    }
//
//    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "pthread_mutex_init", I32, pthread_mutex_init, I32 a, I32 b) {
//        util::getLogger()->debug("S - pthread_mutex_init - {} {}", a, b);
//
//        return 0;
//    }
//
//    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "pthread_mutex_lock", I32, pthread_mutex_lock, I32 a) {
//        util::getLogger()->debug("S - pthread_mutex_lock - {}", a);
//        return 0;
//    }
//
//    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "pthread_mutex_unlock", I32, pthread_mutex_unlock, I32 a) {
//        util::getLogger()->debug("S - pthread_mutex_unlock - {}", a);
//        return 0;
//    }
//
//    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "pthread_mutex_trylock", I32, pthread_mutex_trylock, I32 a) {
//        util::getLogger()->debug("S - pthread_mutex_trylock - {}", a);
//        return 0;
//    }
//
//    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "pthread_setspecific", I32, pthread_setspecific, I32 a, I32 b) {
//        util::getLogger()->debug("S - pthread_setspecific - {} {}", a, b);
//
//        return 0;
//    }

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
