#include "WasmModule.h"

#include <mutex>

#include <WAVM/Platform/Thread.h>
#include <WAVM/Runtime/Runtime.h>
#include <WAVM/Runtime/Intrinsics.h>

#include <util/barrier.h>
#include <util/environment.h>
#include <Runtime/RuntimePrivate.h>

#define OMP_STACK_SIZE (32* 1024)

namespace wasm {

    // Types in accordance with Clang's OpenMP implementation
    namespace kmp {
        enum sched_type : I32 {
            sch_lower = 32, /**< lower bound for unordered values */
            sch_static_chunked = 33,
            sch_static = 34, /**< static unspecialized */
        };

        enum _reduction_method {
            reduction_method_not_defined = 0,
            critical_reduce_block = (1 << 8),
            atomic_reduce_block = (2 << 8),
            tree_reduce_block = (3 << 8),
            empty_reduce_block = (4 << 8)
        };
    }

    // Global variables controlled by level master
    class OMPLevel {
    public:
        const int depth; // Number of nested OpenMP constructs, 0 for serial code
        const int effective_depth; // Number of parallel regions (> 1 thread) above this level
        int max_active_level; // Max number of effective parallel regions allowed from the top
        const int num_threads; // Number of threads of this level
        int wanted_num_threads; // Desired number of thread set by omp_set_num_threads for all future levels
        int pushed_num_threads; // Num threads pushed by compiler, valid for one parallel section, overrides wanted
        std::unique_ptr<util::Barrier> barrier = {}; // Only needed if num_threads > 1
        std::mutex reduceMutex; // Mutex used for reduction data
        // TODO - This implementation limits to one lock for all critical sections at a level.
        // Mention in report (maybe fix looking at the lck address and doing a lookup on it though?)
        std::mutex criticalSection; // Mutex used in critical sections.

        // Defaults set to mimic Clang 9.0.1 behaviour
        OMPLevel() : depth(0), effective_depth(0), max_active_level(1), num_threads(1), wanted_num_threads(-1),
                     pushed_num_threads(-1) {}

        OMPLevel(const OMPLevel *parent, int num_threads) : depth(parent->depth + 1),
                                                            effective_depth(
                                                                    num_threads > 1 ? parent->effective_depth + 1
                                                                                    : parent->effective_depth),
                                                            max_active_level(parent->max_active_level),
                                                            num_threads(num_threads),
                                                            wanted_num_threads(-1),
                                                            pushed_num_threads(-1) {
            if (num_threads > 1) {
                barrier = std::make_unique<util::Barrier>(num_threads);
            }
        }
    };

    OMPLevel deviceSequentialLevel = {};
    thread_local OMPLevel *thisLevel = &deviceSequentialLevel;

    int get_next_level_num_threads(const OMPLevel *currentLevel) {
        // Limits to one thread if we have exceeded maximum parallelism depth
        if (currentLevel->effective_depth >= currentLevel->max_active_level) {
            return 1;
        }

        // Extracts user preference unless compiler has overridden it for this parallel section
        int nextWanted = currentLevel->pushed_num_threads > 0 ? currentLevel->pushed_num_threads
                                                              : currentLevel->wanted_num_threads;

        // Returns user preference if set or device's maximum
        return nextWanted > 0 ? nextWanted : util::getUsableCores();
    }

    std::atomic<int> mike = {0};

    // Thread-local variables for each OMP thread
    thread_local int thisThreadNumber = 0;

    // Arguments passed to spawned threads. Shared at by the level apart from tid
    struct OMPThreadArgs {
        int tid;
        OMPLevel *newLevel;
        struct WasmThreadSpec spec;
    };

    /**
     * Function used to spawn OMP threads. Will be called from within a thread
     * (hence needs to set up its own TLS)
     */
    I64 ompThreadEntryFunc(void *threadArgsPtr) {
        auto threadArgs = reinterpret_cast<OMPThreadArgs *>(threadArgsPtr);

        // Set up OMP TLS
        thisLevel = threadArgs->newLevel;
        thisThreadNumber = threadArgs->tid;

        return getExecutingModule()->executeThread(threadArgs->spec);
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
        return thisLevel->num_threads;
    }

    /**
     * @return the maximum number of threads that can be used to form a new team if a parallel
     * region without a num_threads clause is encountered.
     */
    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "omp_get_max_threads", I32, omp_get_max_threads) {
        util::getLogger()->debug("S - omp_get_max_threads");
        return get_next_level_num_threads(thisLevel);
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "omp_get_level", I32, omp_get_level) {
        util::getLogger()->debug("S - omp_get_level");
        return thisLevel->depth;
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "omp_get_max_active_levels", I32, omp_get_max_active_levels) {
        util::getLogger()->debug("S - omp_get_max_active_levels");
        return thisLevel->max_active_level;
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "omp_set_max_active_levels", void, omp_set_max_active_levels, I32 level) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->debug("S - omp_set_max_active_levels {}", level);
        if (level < 0) {
            logger->warn("Trying to set active level with a negative number {}", level);
            return;
        }
        thisLevel->max_active_level = level;
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

        if (!thisLevel->barrier || thisLevel->num_threads <= 1) {
            return;
        }

        thisLevel->barrier->wait();
    }

    /**
     * Enter code protected by a `critical` construct. This function blocks until the thread can enter the critical section.
     * @param loc  source location information.
     * @param global_tid  global thread number.
     * @param crit identity of the critical section. This could be a pointer to a lock
        associated with the critical section, or some other suitably unique value.
        The lock is not used because Faasm needs to control the locking mechanism for the team.
     */
    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "__kmpc_critical", void, __kmpc_critical, I32 loc, I32 globalTid, I32 crit) {
        util::getLogger()->debug("S - __kmpc_critical {} {} {}", loc, globalTid, crit);
        if (thisLevel->num_threads > 1) {
            thisLevel->criticalSection.lock();
        }
    }

    /**
     * Exits code protected by a `critical` construct, releasing the held lock. This function blocks until the thread can enter the critical section.
     * @param loc  source location information.
     * @param global_tid  global thread number.
     * @param crit compiler lock. See __kmpc_critical for more information
     */
    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "__kmpc_end_critical", void, __kmpc_end_critical, I32 loc, I32 globalTid,
                                   I32 crit) {
        util::getLogger()->debug("S - __kmpc_end_critical {} {} {}", loc, globalTid, crit);
        if (thisLevel->num_threads > 1) {
            thisLevel->criticalSection.unlock();
        }
    }

    /**
     * The omp flush directive identifies a point at which the compiler ensures that all threads in a parallel region
     * have the same view of specified objects in memory. Like clang here we use a fence, but this semantic might
     * not be suited for multitenancy.
     * @param loc Source location info
     */
    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "__kmpc_flush", void, __kmpc_flush, I32 loc) {
        util::getLogger()->debug("S - __kmpc_flush{}", loc);

        // Full memory fence, a bit overkill maybe for Wasm
        __sync_synchronize();

        // Prevent busy waiting like while(flag) #pragma omp flush(flag)
        WAVM::Platform::yieldToAnotherThread();
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
        util::getLogger()->debug("S - __kmpc_push_num_threads {} {} {}", loc, globalTid, numThreads);
        if (numThreads > 0) {
            thisLevel->pushed_num_threads = numThreads;
        }
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "omp_set_num_threads", void, omp_set_num_threads, I32 numThreads) {
        util::getLogger()->debug("S - omp_set_num_threads {}", numThreads);
        if (numThreads > 0) {
            thisLevel->wanted_num_threads = numThreads;
        }
    }

    /**
     * If the runtime is called once, equivalent of calling get_thread_num() at the deepest
     * @param loc The usual...
     * @return
     */
    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "__kmpc_global_thread_num", I32, __kmpc_global_thread_num, I32 loc) {
        util::getLogger()->debug("S - __kmpc_global_thread_num {}", loc);
        return thisThreadNumber; // Might be wrong if called at depth 1 while another thread at depths 1 has forked
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
//        mike++;
//        logger->info("Called {}", mike);

        WasmModule *parentModule = getExecutingModule();
        Runtime::Memory *memoryPtr = parentModule->defaultMemory;
        message::Message *parentCall = getExecutingCall();

        // Retrieve the microstask function from the table
        Runtime::Function *func = Runtime::asFunction(
                Runtime::getTableElement(getExecutingModule()->defaultTable, microtaskPtr));

        // Set up number of threads for next level
        int nextNumThreads = get_next_level_num_threads(thisLevel);
//        int nextNumThreads = 1;
        thisLevel->pushed_num_threads = -1; // Resets for next push

        // Set up new level
        OMPLevel *nextLevel = new OMPLevel(thisLevel, nextNumThreads);

        if (nextNumThreads <= -1) {
            logger->warn("Skipping something");
            OMPLevel *toRestore = thisLevel;
            int oldNumber = thisThreadNumber;
            thisLevel = nextLevel;
            std::vector<IR::UntaggedValue> mta;
            if (argc > 0) {
                U32 *pointers = Runtime::memoryArrayPtr<U32>(memoryPtr, argsPtr, argc);
                // Get pointer to start of arguments in host memory
                for (int argIdx = 0; argIdx < argc; argIdx++) {
                    mta.emplace_back(pointers[argIdx]);
                }
            }
            thisThreadNumber = 1;
            U32 thisStackBase = getExecutingModule()->mmapMemory(OMP_STACK_SIZE);
            U32 stackTop = thisStackBase + OMP_STACK_SIZE - 1;
            Runtime::Context *threadContext = createContext(
                    getCompartmentFromContextRuntimeData(contextRuntimeData)
            );
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
                    func,
                    Runtime::getFunctionType(func),
                    mta.data(),
                    &result
            );

            logger->warn("After");
            thisThreadNumber = oldNumber;
            thisLevel = toRestore;
            return;
        } else if (thisLevel->depth > 1){
            logger->error("FSJKD");
        }
        // Note - must ensure thread arguments are outside loop scope otherwise they do
        // may not exist by the time the thread actually consumes them
        std::vector<OMPThreadArgs> threadArgs;
        threadArgs.reserve(nextNumThreads);

        std::vector<std::vector<IR::UntaggedValue>> microtaskArgs;
        microtaskArgs.reserve(nextNumThreads);

        std::vector<WAVM::Platform::Thread *> platformThreads;
        platformThreads.reserve(nextNumThreads);

        // Build up arguments
        for (int threadNum = 0; threadNum < nextNumThreads; threadNum++) {
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
                                         .tid = threadNum, .newLevel = nextLevel,
                                         .spec.contextRuntimeData = contextRuntimeData,
                                         .spec.parentModule = parentModule, .spec.parentCall = parentCall,
                                         .spec.func = func, .spec.funcArgs = microtaskArgs[threadNum].data(),
                                         .spec.stackSize = OMP_STACK_SIZE
                                 });
        }

        // Create the threads themselves
        for (int threadNum = 0; threadNum < nextNumThreads; threadNum++) {
            platformThreads.emplace_back(Platform::createThread(
                    16 * 1024 * 1024,
//                    0,
                    ompThreadEntryFunc,
                    &threadArgs[threadNum]
            ));
        }

//        for (int threadNum = 0; threadNum < nextNumThreads; threadNum++) {
//           std::thread(
//                    ompThreadEntryFunc,
//                    &threadArgs[threadNum]
//            ).join();
//        }

        // Await all threads
        I64 numErrors = 0;
        for (auto t: platformThreads) {
            numErrors += Platform::joinThread(t);
        }
//        logger->info("joined {}", mike);

        if (numErrors) {
            throw std::runtime_error(fmt::format("{}} OMP threads have exited with errors", numErrors));
        }

        delete nextLevel;
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

        if (thisLevel->num_threads == 1) {
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
            case kmp::sch_static_chunked: {
                int span;
                if (chunk < 1) {
                    chunk = 1;
                }
                span = chunk * incr;
                *stride = span * thisLevel->num_threads;
                *lower = *lower + (span * thisThreadNumber);
                *upper = *lower + span - incr;
                *lastIter = (thisThreadNumber == ((tripCount - 1) / (unsigned int) chunk) % thisLevel->num_threads);
                break;
            }
            case kmp::sch_static: { // (chunk not given)
                // If we have fewer trip_counts than threads
                if (tripCount < thisLevel->num_threads) {
                    logger->warn("Small for loop trip count {} {}", tripCount,
                                 thisLevel->num_threads); // Warns for future use, not tested at scale
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
                    U32 small_chunk = tripCount / thisLevel->num_threads;
                    U32 extras = tripCount % thisLevel->num_threads;
                    *lower += incr * (thisThreadNumber * small_chunk +
                                      (thisThreadNumber < extras ? thisThreadNumber : extras));
                    *upper = *lower + small_chunk * incr - (thisThreadNumber < extras ? 0 : incr);
                    *lastIter = (thisThreadNumber == thisLevel->num_threads - 1);
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
     * There exists many reduction methods, implementing everything as a reduce block
     */
    kmp::_reduction_method determineReductionMethod() {
        if (thisLevel->num_threads == 1) {
            return kmp::empty_reduce_block;
        }
        return kmp::critical_reduce_block;
    }

    /**
     *  When reaching the end of the reduction loop, the threads need to synchronise to operate the
     *  reduction function. In the multi-machine case, this
     */
    int runReduction() {
        int retVal = 0;

        switch (determineReductionMethod()) {
            case kmp::critical_reduce_block:
                util::getLogger()->debug("Thread {} reduction locking", thisThreadNumber);
                thisLevel->reduceMutex.lock();
                retVal = 1;
                break;
            case kmp::empty_reduce_block:
                retVal = 1;
                break;
            case kmp::atomic_reduce_block:
                retVal = 2;
                break;
            case kmp::reduction_method_not_defined:
            case kmp::tree_reduce_block:
                std::runtime_error("Unsupported reduce operation");
        }
        return retVal;
    }

    /**
     *  Called immediately after running the reduction section before exiting the `reduce` construct.
     */
    void endReduction() {
        // Unlocking not owned mutex is UB
        if (thisLevel->num_threads > 1) {
            util::getLogger()->debug("Thread {} unlocking reduction", thisThreadNumber);
            thisLevel->reduceMutex.unlock();
        }
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

        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->debug("S - __kmpc_reduce {} {} {} {} {} {} {}", loc, gtid, num_vars, reduce_size,
                      reduce_data, reduce_func, lck);
        return runReduction();
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
    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "__kmpc_reduce_nowait", I32, __kmpc_reduce_nowait, I32 loc, I32 gtid,
                                   I32 num_vars, I32 reduce_size, I32 reduce_data, I32 reduce_func, I32 lck) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->debug("S - __kmpc_reduce_nowait {} {} {} {} {} {} {}", loc, gtid, num_vars, reduce_size, reduce_data,
                      reduce_func, lck);
        return runReduction();
    }

    /**
     * Finish the execution of a blocking reduce. The lck pointer must be the same as that used in the corresponding start function.
     * @param loc location info
     * @param gtid global thread id
     * @param lck kmp_critical_name* to the critical section.
     */
    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "__kmpc_end_reduce", void, __kmpc_end_reduce, I32 loc, I32 gtid, I32 lck) {
        util::getLogger()->debug("S - __kmpc_end_reduce {} {} {}", loc, gtid, lck);
        endReduction();
    }

    /**
     * Arguments similar to __kmpc_end_reduce. Finish the execution of a reduce_nowait.
     * @param loc
     * @param gtid
     * @param lck
     */
    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "__kmpc_end_reduce_nowait", void, __kmpc_end_reduce_nowait, I32 loc, I32 gtid,
                                   I32 lck) {
        util::getLogger()->debug("S - __kmpc_end_reduce_nowait {} {} {}", loc, gtid, lck);
        endReduction();
    }

    void ompLink() {

    }
}
