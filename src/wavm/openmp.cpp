#include "faabric/util/gids.h"
#include <WAVM/Platform/Thread.h>
#include <WAVM/Runtime/Intrinsics.h>
#include <WAVM/Runtime/Runtime.h>

#include <faabric/scheduler/Scheduler.h>
#include <faabric/state/StateKeyValue.h>
#include <faabric/util/locks.h>
#include <faabric/util/timing.h>

#include <wavm/ThreadState.h>
#include <wavm/WAVMWasmModule.h>

using namespace WAVM;

namespace wasm {

// ------------------------------------------------
// THREAD NUMS AND LEVELS
// ------------------------------------------------

/**
 * @return the thread number, within its team, of the thread executing the
 * function.
 */
WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "omp_get_thread_num",
                               I32,
                               omp_get_thread_num)
{
    faabric::util::getLogger()->debug("S - omp_get_thread_num");
    return getOpenMPContext().threadNumber;
}

/**
 * @return the number of threads currently in the team executing the parallel
 * region from which it is called.
 */
WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "omp_get_num_threads",
                               I32,
                               omp_get_num_threads)
{
    faabric::util::getLogger()->debug("S - omp_get_num_threads");
    return getOpenMPContext().level->numThreads;
}

/**
 * This function returns the max number of threads that can be used in a new
 * team if no num_threads value is provided.
 */
WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "omp_get_max_threads",
                               I32,
                               omp_get_max_threads)
{
    faabric::util::getLogger()->debug("S - omp_get_max_threads");
    return getOpenMPContext().level->getMaxThreadsAtNextLevel();
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env, "omp_get_level", I32, omp_get_level)
{
    faabric::util::getLogger()->debug("S - omp_get_level");
    return getOpenMPContext().level->depth;
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "omp_get_max_active_levels",
                               I32,
                               omp_get_max_active_levels)
{
    faabric::util::getLogger()->debug("S - omp_get_max_active_levels");
    return getOpenMPContext().level->maxActiveLevels;
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "omp_set_max_active_levels",
                               void,
                               omp_set_max_active_levels,
                               I32 level)
{
    const std::shared_ptr<spdlog::logger>& logger = faabric::util::getLogger();
    logger->debug("S - omp_set_max_active_levels {}", level);

    if (level < 0) {
        logger->warn("Trying to set active level with a negative number {}",
                     level);
    } else {
        getOpenMPContext().level->maxActiveLevels = level;
    }
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "__kmpc_push_num_threads",
                               void,
                               __kmpc_push_num_threads,
                               I32 loc,
                               I32 globalTid,
                               I32 numThreads)
{
    faabric::util::getLogger()->debug(
      "S - __kmpc_push_num_threads {} {} {}", loc, globalTid, numThreads);

    if (numThreads > 0) {
        getOpenMPContext().level->pushedThreads = numThreads;
    }
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "omp_set_num_threads",
                               void,
                               omp_set_num_threads,
                               I32 numThreads)
{
    faabric::util::getLogger()->debug("S - omp_set_num_threads {}", numThreads);

    if (numThreads > 0) {
        getOpenMPContext().level->wantedThreads = numThreads;
    }
}

/**
 * If the runtime is called once, equivalent of calling get_thread_num() at the
 * deepest
 * @param loc The usual...
 * @return
 */
WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "__kmpc_global_thread_num",
                               I32,
                               __kmpc_global_thread_num,
                               I32 loc)
{
    faabric::util::getLogger()->debug("S - __kmpc_global_thread_num {}", loc);

    // Might be wrong if called at depth 1 while another thread at depths 1 has
    // forked
    return getOpenMPContext().threadNumber;
}

// ----------------------------------------------------
// BARRIER
// ----------------------------------------------------

/**
 * Synchronization point at which threads in a parallel region will not execute
 * beyond the omp barrier until all other threads in the team complete all
 * explicit tasks in the region. Concepts used for reductions and split
 * barriers.
 * @param loc
 * @param global_tid
 */
WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "__kmpc_barrier",
                               void,
                               __kmpc_barrier,
                               I32 loc,
                               I32 globalTid)
{
    faabric::util::getLogger()->debug(
      "S - __kmpc_barrier {} {}", loc, globalTid);

    auto level = getOpenMPContext().level;

    if (level->numThreads > 1) {
        level->barrier->wait();
    }
}

// ----------------------------------------------------
// CRITICAL
// ----------------------------------------------------

/**
 * Enter code protected by a `critical` construct. This function blocks until
 the thread can enter the critical section.
 * @param loc  source location information.
 * @param global_tid  global thread number.
 * @param crit identity of the critical section. This could be a pointer to a
 lock associated with the critical section, or some other suitably unique value.
    The lock is not used because Faasm needs to control the locking mechanism
 for the team.
 */
WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "__kmpc_critical",
                               void,
                               __kmpc_critical,
                               I32 loc,
                               I32 globalTid,
                               I32 crit)
{
    faabric::util::getLogger()->debug(
      "S - __kmpc_critical {} {} {}", loc, globalTid, crit);

    auto level = getOpenMPContext().level;
    if (level->numThreads > 1) {
        level->criticalSection.lock();
    }
}

/**
 * Exits code protected by a `critical` construct, releasing the held lock. This
 * function blocks until the thread can enter the critical section.
 * @param loc  source location information.
 * @param global_tid  global thread number.
 * @param crit compiler lock. See __kmpc_critical for more information
 */
WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "__kmpc_end_critical",
                               void,
                               __kmpc_end_critical,
                               I32 loc,
                               I32 globalTid,
                               I32 crit)
{
    faabric::util::getLogger()->debug(
      "S - __kmpc_end_critical {} {} {}", loc, globalTid, crit);

    auto level = getOpenMPContext().level;
    if (level->numThreads > 1) {
        level->criticalSection.unlock();
    }
}

/**
 * The omp flush directive identifies a point at which the compiler ensures that
 * all threads in a parallel region have the same view of specified objects in
 * memory. Like clang here we use a fence, but this semantic might not be suited
 * for distributed work. People doing distributed DSM OMP synch the page there.
 * @param loc Source location info
 */
WAVM_DEFINE_INTRINSIC_FUNCTION(env, "__kmpc_flush", void, __kmpc_flush, I32 loc)
{
    faabric::util::getLogger()->debug("S - __kmpc_flush{}", loc);

    // Full memory fence, a bit overkill maybe for Wasm
    __sync_synchronize();
}

// ----------------------------------------------------
// MASTER
// ----------------------------------------------------

/**
 * No implied BARRIER exists on either entry to or exit from the MASTER section.
 * @param loc  source location information.
 * @param global_tid  global thread number.
 * @return 1 if this thread should execute the <tt>master</tt> block, 0
 * otherwise.
 *
 * Faasm: at the moment we only ensure the MASTER section is ran only once but
 * do not handle properly assigning to the master section. Support for better
 * gtid and teams will come. This is called by all threads with same GTID, which
 * is not what the native code does.
 */
WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "__kmpc_master",
                               I32,
                               __kmpc_master,
                               I32 loc,
                               I32 globalTid)
{
    faabric::util::getLogger()->debug(
      "S - __kmpc_master {} {}", loc, globalTid);
    return (I32)getOpenMPContext().threadNumber == 0;
}

/**
 * Only called by the thread executing the master region.
 * @param loc  source location information.
 * @param global_tid  global thread number .
 */
WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "__kmpc_end_master",
                               void,
                               __kmpc_end_master,
                               I32 loc,
                               I32 globalTid)
{
    faabric::util::getLogger()->debug(
      "S - __kmpc_end_master {} {}", loc, globalTid);

    if (getOpenMPContext().threadNumber != 0) {
        throw std::runtime_error("Calling _kmpc_end_master from non-master");
    }
}

// ----------------------------------------------------
// SINGLE
// ----------------------------------------------------

/**
 * Test whether to execute a single construct. There are no implicit barriers in
 the two "single" calls, rather the compiler should introduce an explicit
 barrier if it is required.
 * @param loc
 * @param globalTid
 * @return 1 if this thread should execute the single construct, zero otherwise.
 */
WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "__kmpc_single",
                               I32,
                               __kmpc_single,
                               I32 loc,
                               I32 globalTid)
{
    faabric::util::getLogger()->debug(
      "S - __kmpc_single {} {}", loc, globalTid);

    return (I32)getOpenMPContext().threadNumber == 0;
}

/**
 * See comment on __kmpc_single
 */
WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "__kmpc_end_single",
                               void,
                               __kmpc_end_single,
                               I32 loc,
                               I32 globalTid)
{
    faabric::util::getLogger()->debug(
      "S - __kmpc_end_single {} {}", loc, globalTid);

    if (getOpenMPContext().threadNumber != 0) {
        throw std::runtime_error("Calling _kmpc_end_single from non-master");
    }
}

// ----------------------------------------------------
// FORKING
// ----------------------------------------------------

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
 * 3+. Separate arguments, each of which is a pointer to one of the non-global
 * shared variables
 */
WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "__kmpc_fork_call",
                               void,
                               __kmpc_fork_call,
                               I32 locPtr,
                               I32 argc,
                               I32 microtaskPtr,
                               I32 argsPtr)
{
    auto logger = faabric::util::getLogger();
    logger->debug(
      "S - __kmpc_fork_call {} {} {} {}", locPtr, argc, microtaskPtr, argsPtr);

    WAVMWasmModule* parentModule = getExecutingWAVMModule();
    Runtime::Memory* memoryPtr = parentModule->defaultMemory;
    faabric::Message* parentCall = getExecutingCall();

    // Retrieve the microtask function from the table
    Runtime::Function* func = parentModule->getFunctionFromPtr(microtaskPtr);

    // Set up number of threads for next level
    auto ctx = getOpenMPContext();
    int nextNumThreads = ctx.level->getMaxThreadsAtNextLevel();

    // Reset for next push as the decision has now been made
    ctx.level->pushedThreads = -1;

    auto conf = faabric::util::getSystemConfig();
    if (conf.threadMode == "remote") {
        std::vector<int> chainedThreads;
        chainedThreads.reserve(nextNumThreads);

        std::string activeSnapshotKey;
        size_t threadSnapshotSize;

        uint32_t snapshotId = faabric::util::generateGid();
        activeSnapshotKey = fmt::format("fork_{}", snapshotId);
        threadSnapshotSize = parentModule->snapshotToState(activeSnapshotKey);

        faabric::scheduler::Scheduler& sch = faabric::scheduler::getScheduler();

        const faabric::Message* originalCall = getExecutingCall();
        const std::string origStr =
          faabric::util::funcToString(*originalCall, false);

        U32* nativeArgs =
          Runtime::memoryArrayPtr<U32>(memoryPtr, argsPtr, argc);

        // Make the chained calls
        for (int threadNum = 0; threadNum < nextNumThreads; threadNum++) {
            faabric::Message call = faabric::util::messageFactory(
              originalCall->user(), originalCall->function());

            call.set_isasync(true);

            for (int argIdx = argc - 1; argIdx >= 0; argIdx--) {
                call.add_ompfunctionargs(nativeArgs[argIdx]);
            }

            // Snapshot details
            call.set_snapshotkey(activeSnapshotKey);
            call.set_snapshotsize(threadSnapshotSize);

            call.set_funcptr(microtaskPtr);

            call.set_ompthreadnum(threadNum);
            call.set_ompnumthreads(nextNumThreads);
            call.set_ompdepth(ctx.level->depth);
            call.set_ompeffdepth(ctx.level->activeLevels);
            call.set_ompmal(ctx.level->maxActiveLevels);

            const std::string chainedStr =
              faabric::util::funcToString(call, false);
            sch.callFunction(call);

            logger->debug("Forked thread {} ({}) -> {} {}(*{}) ({})",
                          origStr,
                          faabric::util::getSystemConfig().endpointHost,
                          chainedStr,
                          microtaskPtr,
                          argsPtr,
                          call.scheduledhost());

            chainedThreads[threadNum] = call.id();
        }

        I64 numErrors = 0;

        for (int threadNum = 0; threadNum < nextNumThreads; threadNum++) {
            int callTimeoutMs =
              faabric::util::getSystemConfig().chainedCallTimeout;

            logger->info(
              "Waiting for thread #{} with call id {} with a timeout of {}",
              threadNum,
              chainedThreads[threadNum],
              callTimeoutMs);

            int returnCode = 1;
            try {
                const faabric::Message result = sch.getFunctionResult(
                  chainedThreads[threadNum], callTimeoutMs);
                returnCode = result.returnvalue();
            } catch (faabric::redis::RedisNoResponseException& ex) {
                faabric::util::getLogger()->error(
                  "Timed out waiting for chained call: {}",
                  chainedThreads[threadNum]);
            } catch (std::exception& ex) {
                faabric::util::getLogger()->error(
                  "Non-timeout exception waiting for chained call: {}",
                  ex.what());
            }

            if (returnCode > 0) {
                numErrors++;
            }
        }

        if (numErrors) {
            throw std::runtime_error(fmt::format(
              "{} OpenMP threads have exited with errors", numErrors));
        }

        logger->debug("Distributed OpenMP threads finished successfully");

    } else if (conf.threadMode == "local") {

        // Set up new level
        auto nextLevel = std::make_shared<Level>(ctx.level, nextNumThreads);

        std::vector<std::thread> threads;

        std::vector<U64> results;
        std::mutex resultMutex;

        // Get pointers to shared variables in host memory
        U32* sharedVarsPtr = nullptr;
        if (argc > 0) {
            sharedVarsPtr =
              Runtime::memoryArrayPtr<U32>(memoryPtr, argsPtr, argc);
        }

        // Execute threads locally
        for (int threadNum = 0; threadNum < nextNumThreads; threadNum++) {
            // Be careful here what you pass in with a reference and what you
            // copy
            threads.emplace_back([&results,
                                  &resultMutex,
                                  &nextLevel,
                                  &contextRuntimeData,
                                  &parentModule,
                                  &parentCall,
                                  &sharedVarsPtr,
                                  &argc,
                                  func,
                                  threadNum] {
                auto logger = faabric::util::getLogger();

                // We are now in a new thread so need to set up everything that
                // uses TLS
                setUpOpenMPContext(threadNum, nextLevel);
                setExecutingModule(parentModule);
                setExecutingCall(parentCall);

                // Set up the arguments for the entry function
                std::vector<IR::UntaggedValue> funcArgs = { threadNum, argc };
                if (argc > 0) {
                    // Get pointer to start of arguments in host memory
                    for (int argIdx = 0; argIdx < argc; argIdx++) {
                        funcArgs.emplace_back(sharedVarsPtr[argIdx]);
                    }
                }

                // Arguments for spawning the thread
                WasmThreadSpec spec = {
                    .contextRuntimeData = contextRuntimeData,
                    .func = func,
                    .funcArgs = funcArgs.data(),
                    .stackTop = parentModule->allocateThreadStack(),
                };

                // This executes the thread synchronously
                I64 threadResult = parentModule->executeThreadLocally(spec);

                // Add the result of this thread to the list
                faabric::util::UniqueLock lock(resultMutex);
                results.push_back(threadResult);
            });
        }

        // Await thread completion
        for (auto& t : threads) {
            if (t.joinable()) {
                t.join();
            }
        }

        for (int res : results) {
            if (res < 0) {
                throw std::runtime_error("OMP thread exited with error");
            }
        }
    } else {
        logger->error("Unrecognised thread mode: {}", conf.threadMode);
        throw std::runtime_error("Unrecognised thread mode");
    }
}

// ----------------------------------------------------
// CUSTOM FAASM HOST INTERFACE
// ----------------------------------------------------

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "faasmp_incrby",
                               I64,
                               __faasmp_incrby,
                               I32 keyPtr,
                               I64 value)
{
    const std::shared_ptr<spdlog::logger>& logger = faabric::util::getLogger();
    logger->debug("S - __faasmp_incryby {} {}", keyPtr, value);

    Runtime::Memory* memoryPtr = getExecutingWAVMModule()->defaultMemory;
    std::string key{ &Runtime::memoryRef<char>(memoryPtr, (Uptr)keyPtr) };
    faabric::redis::Redis& redis = faabric::redis::Redis::getState();
    return redis.incrByLong(key, value);
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "__faasmp_getLong",
                               I64,
                               __faasmp_getLong,
                               I32 keyPtr)
{
    const std::shared_ptr<spdlog::logger>& logger = faabric::util::getLogger();
    logger->debug("S - __faasmp_getLong {}", keyPtr);

    Runtime::Memory* memoryPtr = getExecutingWAVMModule()->defaultMemory;
    std::string key{ &Runtime::memoryRef<char>(memoryPtr, (Uptr)keyPtr) };
    faabric::redis::Redis& redis = faabric::redis::Redis::getState();
    return redis.getLong(key);
}

/**
 * This function is just around to debug issues with threaded access to stacks.
 */
WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "__faasmp_debug_copy",
                               void,
                               __faasmp_debug_copy,
                               I32 src,
                               I32 dest)
{
    const std::shared_ptr<spdlog::logger>& logger = faabric::util::getLogger();
    logger->debug("S - __faasmp_debug_copy {} {}", src, dest);

    // Get pointers on host to both src and dest
    Runtime::Memory* memoryPtr = getExecutingWAVMModule()->defaultMemory;
    int* hostSrc = &Runtime::memoryRef<int>(memoryPtr, src);
    int* hostDest = &Runtime::memoryRef<int>(memoryPtr, dest);

    int threadNumber = getOpenMPContext().threadNumber;
    logger->debug("{}: copy {} -> {}", threadNumber, *hostSrc, *hostDest);

    *hostDest = *hostSrc;
}

// -------------------------------------------------------
// FOR LOOP STATIC INIT
// -------------------------------------------------------

enum sched_type : int
{
    sch_lower = 32, /**< lower bound for unordered values */
    sch_static_chunked = 33,
    sch_static = 34, /**< static unspecialized */
};

template<typename T>
void for_static_init(I32 schedule,
                     I32* lastIter,
                     T* lower,
                     T* upper,
                     T* stride,
                     T incr,
                     T chunk);

template<typename T>
void for_static_init(I32 schedule,
                     I32* lastIter,
                     T* lower,
                     T* upper,
                     T* stride,
                     T incr,
                     T chunk)
{
    // Unsigned version of the given template parameter
    typedef typename std::make_unsigned<T>::type UT;

    auto logger = faabric::util::getLogger();
    auto ctx = getOpenMPContext();

    if (ctx.level->numThreads == 1) {
        *lastIter = true;

        if (incr > 0) {
            *stride = *upper - *lower + 1;
        } else {
            *stride = -(*lower - *upper + 1);
        }

        return;
    }

    UT tripCount;
    if (incr == 1) {
        tripCount = *upper - *lower + 1;

    } else if (incr == -1) {
        tripCount = *lower - *upper + 1;

    } else if (incr > 0) {
        // Upper-lower can exceed the limit of signed type
        tripCount = (int)(*upper - *lower) / incr + 1;

    } else {
        tripCount = (int)(*lower - *upper) / (-incr) + 1;
    }

    switch (schedule) {
        case sch_static_chunked: {
            int span;

            if (chunk < 1) {
                chunk = 1;
            }

            span = chunk * incr;

            *stride = span * ctx.level->numThreads;
            *lower = *lower + (span * ctx.threadNumber);
            *upper = *lower + span - incr;

            *lastIter =
              (ctx.threadNumber ==
               ((tripCount - 1) / (unsigned int)chunk) % ctx.level->numThreads);

            break;
        }

        case sch_static: { // (chunk not given)

            // If we have fewer trip_counts than threads
            if (tripCount < ctx.level->numThreads) {
                // Warning for future use, not tested at scale
                logger->warn("Small for loop trip count {} {}",
                             tripCount,
                             ctx.level->numThreads);

                if (ctx.threadNumber < tripCount) {
                    *upper = *lower = *lower + ctx.threadNumber * incr;
                } else {
                    *lower = *upper + incr;
                }

                *lastIter = (ctx.threadNumber == tripCount - 1);

            } else {
                // TODO: We only implement below kmp_sch_static_balanced, not
                // kmp_sch_static_greedy Those are set through KMP_SCHEDULE so
                // we would need to look out for real code setting this
                logger->debug("Ignores KMP_SCHEDULE variable, defaults to "
                              "static balanced schedule");

                U32 small_chunk = tripCount / ctx.level->numThreads;
                U32 extras = tripCount % ctx.level->numThreads;

                *lower += incr * (ctx.threadNumber * small_chunk +
                                  (ctx.threadNumber < extras ? ctx.threadNumber
                                                             : extras));

                *upper = *lower + small_chunk * incr -
                         (ctx.threadNumber < extras ? 0 : incr);

                *lastIter = (ctx.threadNumber == ctx.level->numThreads - 1);
            }

            *stride = tripCount;
            break;
        }
        default: {
            throw std::runtime_error(
              fmt::format("Unimplemented scheduler {}", schedule));
        }
    }
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
 * The functions compute the upper and lower bounds and strides to be used for
 * the set of iterations to be executed by the current thread.
 *
 * The guts of the implementation in openmp can be found in
 * __kmp_for_static_init in runtime/src/kmp_sched.cpp
 *
 * See sched_type for supported scheduling.
 */
WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "__kmpc_for_static_init_4",
                               void,
                               __kmpc_for_static_init_4,
                               I32 loc,
                               I32 gtid,
                               I32 schedule,
                               I32 lastIterPtr,
                               I32 lowerPtr,
                               I32 upperPtr,
                               I32 stridePtr,
                               I32 incr,
                               I32 chunk)
{
    const std::shared_ptr<spdlog::logger>& logger = faabric::util::getLogger();
    logger->debug("S - __kmpc_for_static_init_4 {} {} {} {} {} {} {} {} {}",
                  loc,
                  gtid,
                  schedule,
                  lastIterPtr,
                  lowerPtr,
                  upperPtr,
                  stridePtr,
                  incr,
                  chunk);

    // Get host pointers for the things we need to write
    Runtime::Memory* memoryPtr = getExecutingWAVMModule()->defaultMemory;
    I32* lastIter = &Runtime::memoryRef<I32>(memoryPtr, lastIterPtr);
    I32* lower = &Runtime::memoryRef<I32>(memoryPtr, lowerPtr);
    I32* upper = &Runtime::memoryRef<I32>(memoryPtr, upperPtr);
    I32* stride = &Runtime::memoryRef<I32>(memoryPtr, stridePtr);

    for_static_init<I32>(schedule, lastIter, lower, upper, stride, incr, chunk);
}

/*
 * See __kmpc_for_static_init_4
 */
WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "__kmpc_for_static_init_8",
                               void,
                               __kmpc_for_static_init_8,
                               I32 loc,
                               I32 gtid,
                               I32 schedule,
                               I32 lastIterPtr,
                               I32 lowerPtr,
                               I32 upperPtr,
                               I32 stridePtr, // Pointers to I64
                               I64 incr,
                               I64 chunk)
{

    const std::shared_ptr<spdlog::logger>& logger = faabric::util::getLogger();
    logger->debug("S - __kmpc_for_static_init_4 {} {} {} {} {} {} {} {} {}",
                  loc,
                  gtid,
                  schedule,
                  lastIterPtr,
                  lowerPtr,
                  upperPtr,
                  stridePtr,
                  incr,
                  chunk);

    // Get host pointers for the things we need to write
    Runtime::Memory* memoryPtr = getExecutingWAVMModule()->defaultMemory;
    I32* lastIter = &Runtime::memoryRef<I32>(memoryPtr, lastIterPtr);
    I64* lower = &Runtime::memoryRef<I64>(memoryPtr, lowerPtr);
    I64* upper = &Runtime::memoryRef<I64>(memoryPtr, upperPtr);
    I64* stride = &Runtime::memoryRef<I64>(memoryPtr, stridePtr);

    for_static_init<I64>(schedule, lastIter, lower, upper, stride, incr, chunk);
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "__kmpc_for_static_fini",
                               void,
                               __kmpc_for_static_fini,
                               I32 loc,
                               I32 gtid)
{
    faabric::util::getLogger()->debug(
      "S - __kmpc_for_static_fini {} {}", loc, gtid);
}

// ---------------------------------------------------
// REDUCTION
// ---------------------------------------------------

// Allows reduce return values. See __kmpc_reduce docs (at time of writing team
// atomic not supported)
enum ReduceReturnValue
{
    NON_MASTER_THREAD = 0,
    MASTER_THREAD = 1,
    TEAM_ATMOIC = 2,
};

/*
 * This method should return one of the reduce return values as listed in the
 * OpenMP source.
 */
int startReduction()
{
    auto logger = faabric::util::getLogger();

    auto ctx = getOpenMPContext();

    if (ctx.level->numThreads <= 1) {
        return MASTER_THREAD;
    }

    if (ctx.threadNumber != 0) {
        return NON_MASTER_THREAD;
    }

    // Lock if master with multiple threads
    faabric::util::getLogger()->debug("Locking OMP reduction");
    ctx.level->reduceMutex.lock();
    return MASTER_THREAD;
}

/**
 *  Called immediately after running the reduction section before exiting the
 * `reduce` construct.
 */
void endReduction()
{
    auto ctx = getOpenMPContext();

    if (ctx.level->numThreads <= 1 || ctx.threadNumber != 0) {
        return;
    }

    faabric::util::getLogger()->debug("Unlocking OMP reduction");
    ctx.level->reduceMutex.unlock();
}

/**
 * A blocking reduce that includes an implicit barrier.
 * @param loc source location information
 * @param gtid global thread id
 * @param num_vars number of items (variables) to be reduced
 * @param reduce_size size of data in bytes to be reduced
 * @param reduce_data pointer to data to be reduced
 * @param reduce_func callback function providing reduction operation on two
 * operands and returning result of reduction in lhs_data. Of type void(*)(void
 * *lhs data, void *rhs data)
 * @param lck pointer to the unique lock data structure
 * @return 1 for the master thread, 0 for all other team threads, 2 for all team
 * threads if atomic reduction needed
 */
WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "__kmpc_reduce",
                               I32,
                               __kmpc_reduce,
                               I32 loc,
                               I32 gtid,
                               I32 numVars,
                               I32 reduceSize,
                               I32 reduceData,
                               I32 reduceFunc,
                               I32 lck)
{
    faabric::util::getLogger()->debug("S - __kmpc_reduce {} {} {} {} {} {} {}",
                                      loc,
                                      gtid,
                                      numVars,
                                      reduceSize,
                                      reduceData,
                                      reduceFunc,
                                      lck);

    return startReduction();
}

/**
 * The nowait version is used for a reduce clause with the nowait argument. Or
 * direct exit of parallel section.
 * @param loc source location information
 * @param gtid global thread id
 * @param num_vars number of items (variables) to be reduced
 * @param reduce_size size of data in bytes to be reduced
 * @param reduce_data pointer to data to be reduced
 * @param reduce_func callback function providing reduction operation on two
 * operands and returning result of reduction in lhs_data. Of type void(*)(void
 * *lhs_data, void *rhs_data)
 * @param lck pointer to the unique lock data structure
 * @return 1 for the master thread, 0 for all other team threads, 2 for all team
 * threads if atomic reduction needed
 */
WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "__kmpc_reduce_nowait",
                               I32,
                               __kmpc_reduce_nowait,
                               I32 loc,
                               I32 gtid,
                               I32 numVars,
                               I32 reduceSize,
                               I32 reduceData,
                               I32 reduceFunc,
                               I32 lck)
{
    faabric::util::getLogger()->debug(
      "S - __kmpc_reduce_nowait {} {} {} {} {} {} {}",
      loc,
      gtid,
      numVars,
      reduceSize,
      reduceData,
      reduceFunc,
      lck);

    return startReduction();
}

/**
 * Finish the execution of a blocking reduce. The lck pointer must be the same
 * as that used in the corresponding start function.
 * @param loc location info
 * @param gtid global thread id
 * @param lck kmp_critical_name* to the critical section.
 */
WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "__kmpc_end_reduce",
                               void,
                               __kmpc_end_reduce,
                               I32 loc,
                               I32 gtid,
                               I32 lck)
{
    faabric::util::getLogger()->debug(
      "S - __kmpc_end_reduce {} {} {}", loc, gtid, lck);

    endReduction();
}

/**
 * Arguments similar to __kmpc_end_reduce. Finish the execution of a
 * reduce_nowait.
 * @param loc
 * @param gtid
 * @param lck
 */
WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "__kmpc_end_reduce_nowait",
                               void,
                               __kmpc_end_reduce_nowait,
                               I32 loc,
                               I32 gtid,
                               I32 lck)
{
    faabric::util::getLogger()->debug(
      "S - __kmpc_end_reduce_nowait {} {} {}", loc, gtid, lck);

    endReduction();
}

// ----------------------------------------------
// DEVICES
// ----------------------------------------------

/**
 * Get the number of devices (different CPU sockets or machines) available to
 * that user
 */
WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "omp_get_num_devices",
                               int,
                               omp_get_num_devices)
{
    faabric::util::getLogger()->debug("S - omp_get_num_devices");
    return 1;
}

/**
 * Switches between local and remote threads.
 */
WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "omp_set_default_device",
                               void,
                               omp_set_default_device,
                               int defaultDeviceNumber)
{
    auto logger = faabric::util::getLogger();
    logger->debug("S - omp_set_default_device {} (ignored)",
                  defaultDeviceNumber);
}

void ompLink() {}
}
