#include "WasmModule.h"

#include <WAVM/Platform/Thread.h>
#include <WAVM/Runtime/Runtime.h>
#include <WAVM/Runtime/Intrinsics.h>

#include <sys/mman.h>
#include <util/barrier.h>
#include <util/environment.h>
#include <Runtime/RuntimePrivate.h>

#define OMP_STACK_SIZE 2 * ONE_MB_BYTES

namespace wasm {
    // Thread-local variables for each OMP thread
    static thread_local int thisThreadNumber = 0;
    static thread_local U32 thisStackBase = -1;

    // Global variables controlled by master
    static int numThreadsOverride = -1;
    static unsigned int sectionThreadCount = 1;

    // Locking/ barriers
    static util::Barrier *activeBarrier;

    // Threads currently in action
    std::vector<WAVM::Platform::Thread *> platformThreads;

    // Arguments passed to spawned threads
    struct OMPThreadArgs {
        int tid;
        Runtime::ContextRuntimeData *contextRuntimeData;
        wasm::WasmModule *parentModule;
        message::Message *parentCall;
        Runtime::Function *func;
        IR::UntaggedValue *funcArgs;
    };

    void resetOpenMP() {
        // Clear thread references and thread number override
        platformThreads.clear();

        // Reset number threads override
        numThreadsOverride = -1;
        sectionThreadCount = 1;

        // Clear the active barrier
        delete activeBarrier;
    }

    /**
     * Function used to spawn OMP threads. Will be called from within a thread
     * (hence needs to set up its own TLS)
     */
    I64 ompThreadEntryFunc(void *threadArgsPtr) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        auto threadArgs = reinterpret_cast<OMPThreadArgs *>(threadArgsPtr);
        wasm::setExecutingModule(threadArgs->parentModule);
        wasm::setExecutingCall(threadArgs->parentCall);

        // Set up TLS
        thisThreadNumber = threadArgs->tid;

        // Create a new region for this thread's stack
        thisStackBase = getExecutingModule()->mmapMemory(OMP_STACK_SIZE);
        U32 stackTop = thisStackBase + OMP_STACK_SIZE - 1;

        // Create a new context for this thread
        Runtime::Context *threadContext = createContext(
                getCompartmentFromContextRuntimeData(threadArgs->contextRuntimeData)
        );

        // Set the stack pointer in this context
        IR::UntaggedValue &stackGlobal = threadContext->runtimeData->mutableGlobals[0];
        if (stackGlobal.u32 != STACK_SIZE) {
            logger->error("Expected first mutable global in context to be stack pointer ({})", stackGlobal.u32);
            throw std::runtime_error("Unexpected mutable global format");
        }

        threadContext->runtimeData->mutableGlobals[0] = stackTop;

        // Execute the function
        IR::UntaggedValue result;
        Runtime::invokeFunction(
                threadContext,
                threadArgs->func,
                Runtime::getFunctionType(threadArgs->func),
                threadArgs->funcArgs,
                &result
        );

        return result.i64;
    }

    /**
     * @return the thread number, within its team, of the thread executing the function.
     */
    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "omp_get_thread_num", I32, omp_get_thread_num) {
        util::getLogger()->debug("S - omp_get_thread_num");
        return thisThreadNumber;
    }

    /**
     * @return the number of threads currently in the team executing the parallel region from
     * which it is called.
     */
    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "omp_get_num_threads", I32, omp_get_num_threads) {
        util::getLogger()->debug("S - omp_get_num_threads");
        return sectionThreadCount;
    }

    /**
     * @return the maximum number of threads that can be used to form a new team if a parallel
     * region without a num_threads clause is encountered.
     */
    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "omp_get_max_threads", I32, omp_get_max_threads) {
        util::getLogger()->debug("S - omp_get_max_threads");
        return util::getUsableCores();
    }

    /**
     * Synchronization point at which threads in a parallel region will not execute beyond
     * the omp barrier until all other threads in the team complete all explicit tasks in the region.
     * Concepts used for reductions and split barriers.
     * @param loc
     * @param global_tid
     */
    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "__kmpc_barrier", void, __kmpc_barrier, I32 loc, I32 globalTid) {
        util::getLogger()->debug("S - __kmpc_barrier {} {}", loc, globalTid);

        if (sectionThreadCount <= 1) {
            return;
        }

        if (activeBarrier == nullptr) {
            throw std::runtime_error("No active barrier");
        }

        activeBarrier->wait();
    }

    /**
     * No implied BARRIER exists on either entry to or exit from the MASTER section.
     * @param loc  source location information.
     * @param global_tid  global thread number.
     * @return 1 if this thread should execute the <tt>master</tt> block, 0 otherwise.
     *
     * Faasm: at the moment we only ensure the MASTER section is ran only once but do
     * not handle properly assigning to the master section. Support for better gtid and
     * teams will come. This is called by all threads with same GTID, which is not
     * what the native code does.
     */
    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "__kmpc_master", I32, __kmpc_master, I32 loc, I32 globalTid) {
        util::getLogger()->debug("S - __kmpc_master {} {}", loc, globalTid);
        return thisThreadNumber == 0 ? 1 : 0;
    }

    /**
     * Only called by the thread executing the master region.
     * @param loc  source location information.
     * @param global_tid  global thread number .
     */
    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "__kmpc_end_master", void, __kmpc_end_master, I32 loc, I32 globalTid) {
        util::getLogger()->debug("S - __kmpc_end_master {} {}", loc, globalTid);
        WAVM_ASSERT(globalTid == 0 && thisThreadNumber == 0)
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "__kmpc_push_num_threads", void, __kmpc_push_num_threads,
                                   I32 loc, I32 globalTid, I32 numThreads) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->debug("S - __kmpc_push_num_threads {} {} {}", loc, globalTid, numThreads);

        if (numThreads > 0) {
            numThreadsOverride = numThreads;
        }
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "__kmpc_global_thread_num", I32, __kmpc_global_thread_num, I32 loc) {
        util::getLogger()->debug("S - __kmpc_global_thread_num {}", loc);
        return thisThreadNumber;
    }

    /**
     * The "real" version of this function is implemented in the openmp source at
     * openmp/runtime/src/kmp_csupport.cpp. This in turn calls __kmp_fork_call which
     * does the real heavy lifting (see openmp/runtime/src/kmp_runtime.cpp)
     *
     * @param locPtr pointer to the source location info (type ident_t)
     * @param argc number of arguments to pass to the microtask
     * @param microtaskPtr function pointer for the microtask itself (microtask_t)
     * @param argsPtr pointer to the arguments for the microtask (if applicable)
     *
     * The microtask function takes two or more arguments:
     * 1. The thread ID within its current team
     * 2. The number of non-global shared variables it has access to
     * 3+. Separate arguments, each of which is a pointer to one of the non-global shared variables
     */
    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "__kmpc_fork_call", void, __kmpc_fork_call, I32 locPtr, I32 argc,
                                   I32 microtaskPtr, I32 argsPtr) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->debug("S - __kmpc_fork_call {} {} {} {}", locPtr, argc, microtaskPtr, argsPtr);

        Runtime::Memory *memoryPtr = getExecutingModule()->defaultMemory;
        WasmModule *parentModule = getExecutingModule();
        message::Message *parentCall = getExecutingCall();

        // Retrieve the microstask function from the table
        Runtime::Function *func = Runtime::asFunction(
                Runtime::getTableElement(getExecutingModule()->defaultTable, microtaskPtr));

        // Set up number of threads
        int numThreads = numThreadsOverride > 0 ? numThreadsOverride : util::getUsableCores();
        sectionThreadCount = numThreads;

        // Create barrier in case it's needed
        activeBarrier = new util::Barrier(numThreads);

        // Note - must ensure thread arguments are outside loop scope otherwise they do
        // may not exist by the time the thread actually consumes them
        std::vector<OMPThreadArgs> threadArgs;
        threadArgs.reserve(numThreads);

        std::vector<std::vector<IR::UntaggedValue>> microtaskArgs;
        microtaskArgs.reserve(numThreads);

        // Build up arguments
        for (int threadNum = 0; threadNum < numThreads; threadNum++) {
            // Note - these arguments are the thread number followed by the number of
            // shared variables, then the pointers to those shared variables
            microtaskArgs.push_back({threadNum, argc});
            if (argc > 0) {
                // Get pointer to start of arguments in host memory
                U32 *pointers = Runtime::memoryArrayPtr<U32>(memoryPtr, argsPtr, argc);
                for (int argIdx = 0; argIdx < argc; argIdx++) {
                    microtaskArgs[threadNum].emplace_back(pointers[argIdx]);
                }
            }

            // Arguments for spawning the thread
            // NOTE - CLion auto-format insists on this layout...
            threadArgs.push_back({
                                         .tid = threadNum, .contextRuntimeData = contextRuntimeData,
                                         .parentModule = parentModule, .parentCall = parentCall,
                                         .func = func, .funcArgs = microtaskArgs[threadNum].data()
                                 });
        }

        // Create the threads themselves
        for (int threadNum = 0; threadNum < numThreads; threadNum++) {
            platformThreads.emplace_back(Platform::createThread(
                    0,
                    ompThreadEntryFunc,
                    &threadArgs[threadNum]
            ));
        }

        // Await all threads
        I64 numErrors = 0;
        for (auto t: platformThreads) {
            numErrors += Platform::joinThread(t);
        }

        if (numErrors) {
            throw std::runtime_error(fmt::format("{}} OMP threads have exited with errors", numErrors));
        }

        resetOpenMP();
    }

    /**
     * This function is just around to debug issues with threaded access to stacks.
     */
    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "__faasmp_debug_copy", void, __faasmp_debug_copy, I32 src, I32 dest) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->debug("S - __faasmp_debug_copy {} {}", src, dest);

        // Get pointers on host to both src and dest
        Runtime::Memory *memoryPtr = getExecutingModule()->defaultMemory;
        int *hostSrc = &Runtime::memoryRef<int>(memoryPtr, src);
        int *hostDest = &Runtime::memoryRef<int>(memoryPtr, dest);

        logger->debug("{}: copy {} -> {}", thisThreadNumber, *hostSrc, *hostDest);

        *hostDest = *hostSrc;
    }

    void ompLink() {

    }
}
