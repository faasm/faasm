#include "WasmModule.h"

#include <WAVM/Platform/Thread.h>
#include <WAVM/Runtime/Runtime.h>
#include <WAVM/Runtime/Intrinsics.h>

#include <util/barrier.h>
#include <util/environment.h>

namespace wasm {
    enum sched_type : I32 {
        kmp_sch_lower = 32, /**< lower bound for unordered values */
        kmp_sch_static_chunked = 33,
        kmp_sch_static = 34, /**< static unspecialized */
    };

    // Thread-local variables for each OMP thread
    static thread_local int thisThreadNumber = 0;

    // Global variables controlled by master
    static int numThreadsOverride = -1;
    static unsigned int sectionThreadCount = 1;

    // Locking/ barriers
    static util::Barrier* activeBarrier;

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
        auto threadArgs = reinterpret_cast<OMPThreadArgs *>(threadArgsPtr);
        wasm::setExecutingModule(threadArgs->parentModule);
        wasm::setExecutingCall(threadArgs->parentCall);

        // Set up TLS
        thisThreadNumber = threadArgs->tid;

        // Create a new context for this thread
        auto threadContext = createContext(getCompartmentFromContextRuntimeData(threadArgs->contextRuntimeData));

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

        if(activeBarrier == nullptr) {
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
    /**
   *
   * @param contextRuntimeData
   * @param num_threads
   */
    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "omp_set_num_threads ", void, omp_set_num_threads, I32 num_threads) {
        util::getLogger()->debug("S - omp_set_num_threads {}", num_threads);
        if (num_threads > 0) {
            userNumThread = num_threads;
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
                    THREAD_STACK_SIZE,
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
     * @param    loc       Source code location
     * @param    gtid      Global thread id of this thread
     * @param    schedule  Scheduling type for the parallel loop
     * @param    plastiter Pointer to the "last iteration" flag
     * @param    plower    Pointer to the lower bound
     * @param    pupper    Pointer to the upper bound of loop chunk
     * @param    pstride   Pointer to the stride for parallel loop
     * @param    incr      Loop increment
     * @param    chunk     The chunk size for the parallel loop
     *
     * The functions compute the upper and lower bounds and strides to be used for the
     * set of iterations to be executed by the current thread from the statically
     * scheduled loop that is described by the initial values of the bounds, strides,
     * increment and chunks for parallel loop and distribute constructs.
     *
     * See sched_type for supported scheduling.
     *
     * SAFETY: The implementation here is not thread safe at the moment. The pointers values, although set
     * to the correct values for each thread right before calling this function (on native, COW, so this
     * function's modifications to them should be thread-local but are not at the moment). This function will
     * therefore only work if each thread's work between init and fini is mutually exclusive..
     * TODO: Find a way to make this thread safe, the hard bit is that the compiler assumes that those
     * variables, although at the same address, will be thread-local.
    */
    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "__kmpc_for_static_init_4", void, __kmpc_for_static_init_4,
                                   I32 loc, I32 gtid, I32 schedule, I32 _plastiter, I32 _plower,
                                   I32 _pupper, I32 _pstride, I32 incr, I32 chunk) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->debug("S - __kmpc_for_static_init_4 {} {} {} {} {} {} {} {} {}",
                      loc, gtid, schedule, _plastiter, _plower, _pupper, _pstride, incr, chunk);

        // Assumes the pointers will be valid throughout the function
        Runtime::Memory *memoryPtr = getExecutingModule()->defaultMemory;
        I32 *plower = &Runtime::memoryRef<I32>(memoryPtr, _plower);
        I32 *pupper = &Runtime::memoryRef<I32>(memoryPtr, _pupper);
        I32 *pstride = &Runtime::memoryRef<I32>(memoryPtr, _pstride);
        I32 *plastiter = &Runtime::memoryRef<I32>(memoryPtr, _plastiter);
//        logger->debug("Thread {}: lower {}, upper {}, lastiter {}, stride {}", thisThreadNumber, *plower, *pupper,
//                      *plastiter, *pstride);

        if (thisSectionThreadCount == 1) {
            *plastiter = true;
            *pstride = (incr > 0) ? (*pupper - *plower + 1) : (-(*plower - *pupper + 1));
            return;
        }

        unsigned int trip_count;
        if (incr == 1) {
            trip_count = *pupper - *plower + 1;
        } else if (incr == -1) {
            trip_count = *plower - *pupper + 1;
        } else if (incr > 0) {
            // upper-lower can exceed the limit of signed type
            trip_count = (int) (*pupper - *plower) / incr + 1;
        } else {
            trip_count = (int) (*plower - *pupper) / (-incr) + 1;
        }
//        logger->info("Thread {}: tripcount {}, lower {}, upper {}", thisThreadNumber, trip_count, *plower, *pupper);

        switch (schedule) {
            case 33: { // kmp_sch_static_chunked
                int span;
                if (chunk < 1) {
                    chunk = 1;
                }
                span = chunk * incr;
                *pstride = span * thisSectionThreadCount;
                *plower = *plower + (span * thisThreadNumber);
                *pupper = *plower + span - incr;
                *plastiter = (thisThreadNumber == ((trip_count - 1) / (unsigned int) chunk) % thisSectionThreadCount);
                break;
            }
            case 34: { // kmp_sch_static (chunk not given)
                // If we have fewer trip_counts than threads
                if (trip_count < thisSectionThreadCount) {
                    logger->warn("Small for loop trip count {} {}", trip_count,
                                 thisSectionThreadCount); // Warns for future use, not tested at scale
                    if (thisThreadNumber < trip_count) {
                        *pupper = *plower = *plower + thisThreadNumber * incr;
                    } else {
                        *plower = *pupper + incr;
                    }
                    *plastiter = (thisThreadNumber == trip_count - 1);
                } else {
                    // TODO: We only implement below kmp_sch_static_balanced, not kmp_sch_static_greedy
                    // Those are set through KMP_SCHEDULE so we would need to look out for real code setting this
                    logger->debug("Ignores KMP_SCHEDULE variable, defaults to static balanced schedule");
                    U32 small_chunk = trip_count / thisSectionThreadCount;
                    U32 extras = trip_count % thisSectionThreadCount;
                    *plower += incr * (thisThreadNumber * small_chunk +
                                       (thisThreadNumber < extras ? thisThreadNumber : extras));
                    *pupper = *plower + small_chunk * incr - (thisThreadNumber < extras ? 0 : incr);
                    *plastiter = (thisThreadNumber == thisSectionThreadCount - 1);
                }

                *pstride = trip_count;
                break;
            }

        }
//        logger->debug("After TID{}- lower {}, upper {}, stride {}, lastiter {}", thisThreadNumber, *plower, *pupper,
//                      *pstride, *plastiter);
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "__kmpc_for_static_fini", void, __kmpc_for_static_fini,
                                   I32 loc, I32 gtid) {
        util::getLogger()->debug("S - __kmpc_for_static_fini {} {}", loc, gtid);
    }

    void ompLink() {

    }
}
