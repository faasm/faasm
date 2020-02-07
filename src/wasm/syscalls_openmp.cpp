#include "WasmModule.h"

#include <WAVM/Platform/Thread.h>
#include <WAVM/Runtime/Runtime.h>
#include <WAVM/Runtime/Intrinsics.h>

#include <util/barrier.h>
#include <util/environment.h>
#include <Runtime/RuntimePrivate.h>

#define OMP_STACK_SIZE 2 * ONE_MB_BYTES

namespace wasm {
    enum sched_type : I32 {
        kmp_sch_lower = 32, /**< lower bound for unordered values */
        kmp_sch_static_chunked = 33,
        kmp_sch_static = 34, /**< static unspecialized */
    };

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

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "omp_set_num_threads ", void, omp_set_num_threads, I32 numThreads) {
        util::getLogger()->debug("S - omp_set_num_threads {}", numThreads);
        if (numThreads > 0) {
            // TODO - make this user-specific
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

    /**
     * @param    loc       Source code location
     * @param    gtid      Global thread id of this thread
     * @param    schedule  Scheduling type for the parallel loop
     * @param    lastIterPtr Pointer to the "last iteration" flag (boolean)
     * @param    lowerPtr    Pointer to the lower bound
     * @param    upperPtr    Pointer to the upper bound of loop chunk
     * @param    stridePtr   Pointer to the stride for parallel loop
     * @param    incr      Loop increment
     * @param    chunk     The chunk size for the parallel loop
     *
     * The functions compute the upper and lower bounds and strides to be used for the
     * set of iterations to be executed by the current thread.
     *
     * The guts of the implementation in openmp can be found in __kmp_for_static_init in
     * runtime/src/kmp_sched.cpp
     *
     * See sched_type for supported scheduling.
     */
    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "__kmpc_for_static_init_4", void, __kmpc_for_static_init_4,
                                   I32 loc, I32 gtid, I32 schedule, I32 lastIterPtr, I32 lowerPtr,
                                   I32 upperPtr, I32 stridePtr, I32 incr, I32 chunk) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->debug("S - __kmpc_for_static_init_4 {} {} {} {} {} {} {} {} {}",
                      loc, gtid, schedule, lastIterPtr, lowerPtr, upperPtr, stridePtr, incr, chunk);

        // Get host pointers for the things we need to write
        Runtime::Memory *memoryPtr = getExecutingModule()->defaultMemory;
        I32 *lastIter = &Runtime::memoryRef<I32>(memoryPtr, lastIterPtr);
        I32 *lower = &Runtime::memoryRef<I32>(memoryPtr, lowerPtr);
        I32 *upper = &Runtime::memoryRef<I32>(memoryPtr, upperPtr);
        I32 *stride = &Runtime::memoryRef<I32>(memoryPtr, stridePtr);

        if (sectionThreadCount == 1) {
            *lastIter = true;
            *stride = (incr > 0) ? (*upper - *lower + 1) : (-(*lower - *upper + 1));
            return;
        }

        unsigned int tripCount;
        if (incr == 1) {
            tripCount = *upper - *lower + 1;
        } else if (incr == -1) {
            tripCount = *lower - *upper + 1;
        } else if (incr > 0) {
            // upper-lower can exceed the limit of signed type
            tripCount = (int) (*upper - *lower) / incr + 1;
        } else {
            tripCount = (int) (*lower - *upper) / (-incr) + 1;
        }

        switch (schedule) {
            case 33: { // kmp_sch_static_chunked
                int span;
                if (chunk < 1) {
                    chunk = 1;
                }
                span = chunk * incr;
                *stride = span * sectionThreadCount;
                *lower = *lower + (span * thisThreadNumber);
                *upper = *lower + span - incr;
                *lastIter = (thisThreadNumber == ((tripCount - 1) / (unsigned int) chunk) % sectionThreadCount);
                break;
            }
            case 34: { // kmp_sch_static (chunk not given)
                // If we have fewer trip_counts than threads
                if (tripCount < sectionThreadCount) {
                    logger->warn("Small for loop trip count {} {}", tripCount,
                                 sectionThreadCount); // Warns for future use, not tested at scale
                    if (thisThreadNumber < tripCount) {
                        *upper = *lower = *lower + thisThreadNumber * incr;
                    } else {
                        *lower = *upper + incr;
                    }
                    *lastIter = (thisThreadNumber == tripCount - 1);
                } else {
                    // TODO: We only implement below kmp_sch_static_balanced, not kmp_sch_static_greedy
                    // Those are set through KMP_SCHEDULE so we would need to look out for real code setting this
                    logger->debug("Ignores KMP_SCHEDULE variable, defaults to static balanced schedule");
                    U32 small_chunk = tripCount / sectionThreadCount;
                    U32 extras = tripCount % sectionThreadCount;
                    *lower += incr * (thisThreadNumber * small_chunk +
                                       (thisThreadNumber < extras ? thisThreadNumber : extras));
                    *upper = *lower + small_chunk * incr - (thisThreadNumber < extras ? 0 : incr);
                    *lastIter = (thisThreadNumber == sectionThreadCount - 1);
                }

                *stride = tripCount;
                break;
            }
            default: {
                throw std::runtime_error(fmt::format("Unimplemented scheduler {}", schedule));
            }
        }
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "__kmpc_for_static_fini", void, __kmpc_for_static_fini,
                                   I32 loc, I32 gtid) {
        util::getLogger()->debug("S - __kmpc_for_static_fini {} {}", loc, gtid);
    }

    /**
     * A blocking reduce that includes an implicit barrier.
     * @param loc source location information
     * @param gtid global thread id
     * @param num_vars number of items (variables) to be reduced
     * @param reduce_size size of data in bytes to be reduced
     * @param reduce_data pointer to data to be reduced
     * @param reduce_func callback function providing reduction operation on two operands and returning result of reduction in lhs_data. Of type void(∗)(void ∗lhs data, void ∗rhs data)
     * @param lck pointer to the unique lock data structure
     * @return 1 for the master thread, 0 for all other team threads, 2 for all team threads if atomic reduction needed
     */
    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "__kmpc_reduce", I32, __kmpc_reduce, I32 loc, I32 gtid, I32 num_vars,
                                   I32 reduce_size, I32 reduce_data, I32 reduce_func, I32 lck) {
        util::getLogger()->debug("S - __kmpc_reduce {} {} {} {} {} {} {}", loc, gtid, num_vars, reduce_size,
                                 reduce_data, reduce_func, lck);
        if (thisThreadNumber == 0) {
            return 1;
        }
        else {
            return 0;
        }
    }

    /**
     * The nowait version is used for a reduce clause with the nowait argument. Or direct exit of parallel section.
     * @param loc source location information
     * @param gtid global thread id
     * @param num_vars number of items (variables) to be reduced
     * @param reduce_size size of data in bytes to be reduced
     * @param reduce_data pointer to data to be reduced
     * @param reduce_func callback function providing reduction operation on two operands and returning result of reduction in lhs_data. Of type void(∗)(void ∗lhs data, void ∗rhs data)
     * @param lck pointer to the unique lock data structure
     * @return 1 for the master thread, 0 for all other team threads, 2 for all team threads if atomic reduction needed
     */
    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "__kmpc_reduce_nowait", I32, __kmpc_reduce_nowait, I32 loc, I32 gtid, I32 num_vars, I32 reduce_size, I32 reduce_data, I32 reduce_func, I32 lck) {
        util::getLogger()->debug("S - __kmpc_reduce_nowait {} {} {} {} {} {} {}", loc, gtid, num_vars, reduce_size, reduce_data, reduce_func, lck);
        if (thisThreadNumber == 0) {
            return 1;
        }
        else {
            return 0;
        }
    }

    /**
     * Finish the execution of a blocking reduce. The lck pointer must be the same as that used in the corresponding start function.
     * @param loc location info
     * @param gtid global thread id
     * @param lck kmp_critical_name* to the critical section.
     */
    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "__kmpc_end_reduce", void, __kmpc_end_reduce, I32 loc, I32 gtid, I32 lck) {
        util::getLogger()->debug("S - __kmpc_end_reduce {} {} {}", loc, gtid, lck);
    }

    /**
     * Arguments similar to __kmpc_end_reduce. Finish the execution of a reduce_nowait.
     * @param loc
     * @param gtid
     * @param lck
     */
    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "__kmpc_end_reduce_nowait", void, __kmpc_end_reduce_nowait, I32 loc, I32 gtid, I32 lck) {
        util::getLogger()->debug("S - __kmpc_end_reduce_nowait {} {} {}", loc, gtid, lck);
    }

    void ompLink() {

    }
}
