#include <WAVM/Platform/Thread.h>
#include <WAVM/Runtime/Intrinsics.h>
#include <WAVM/Runtime/Runtime.h>

#include <faabric/proto/faabric.pb.h>
#include <faabric/scheduler/Scheduler.h>
#include <faabric/snapshot/SnapshotRegistry.h>
#include <faabric/state/StateKeyValue.h>
#include <faabric/util/func.h>
#include <faabric/util/gids.h>
#include <faabric/util/locks.h>
#include <faabric/util/macros.h>
#include <faabric/util/timing.h>
#include <threads/ThreadState.h>
#include <wavm/WAVMWasmModule.h>

using namespace WAVM;

namespace wasm {

// ------------------------------------------------
// LOGGING
// ------------------------------------------------

#define OMP_FUNC(str)                                                          \
    std::shared_ptr<threads::Level> level = threads::getCurrentOpenMPLevel();  \
    faabric::Message* msg = getExecutingCall();                                \
    auto logger = faabric::util::getLogger();                                  \
    logger->trace("OMP {} ({}): " str, msg->appindex(), ::gettid());

#define OMP_FUNC_ARGS(formatStr, ...)                                          \
    std::shared_ptr<threads::Level> level = threads::getCurrentOpenMPLevel();  \
    faabric::Message* msg = getExecutingCall();                                \
    auto logger = faabric::util::getLogger();                                  \
    logger->trace(                                                             \
      "OMP {} ({}): " formatStr, msg->appindex(), gettid(), __VA_ARGS__);

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
    OMP_FUNC("omp_get_thread_num")
    return msg->appindex();
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
    OMP_FUNC("omp_get_num_threads")
    return level->numThreads;
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
    OMP_FUNC("omp_get_max_threads");
    return level->getMaxThreadsAtNextLevel();
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env, "omp_get_level", I32, omp_get_level)
{
    OMP_FUNC("omp_get_level");
    return level->depth;
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "omp_get_max_active_levels",
                               I32,
                               omp_get_max_active_levels)
{
    OMP_FUNC("omp_get_max_active_levels");
    return level->maxActiveLevels;
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "omp_set_max_active_levels",
                               void,
                               omp_set_max_active_levels,
                               I32 maxLevels)
{
    OMP_FUNC_ARGS("omp_set_max_active_levels {}", maxLevels)

    if (level < 0) {
        logger->warn("Trying to set active level with a negative number {}",
                     level);
    } else {
        level->maxActiveLevels = maxLevels;
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
    OMP_FUNC_ARGS(
      "__kmpc_push_num_threads {} {} {}", loc, globalTid, numThreads);

    if (numThreads > 0) {
        level->pushedThreads = numThreads;
    }
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "omp_set_num_threads",
                               void,
                               omp_set_num_threads,
                               I32 numThreads)
{
    OMP_FUNC_ARGS("omp_set_num_threads {}", numThreads);

    if (numThreads > 0) {
        level->wantedThreads = numThreads;
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
    OMP_FUNC_ARGS("__kmpc_global_thread_num {}", loc);

    // Might be wrong if called at depth 1 while another thread at depths 1 has
    // forked
    return msg->appindex();
}

// ------------------------------------------------
// TIMING
// ------------------------------------------------

WAVM_DEFINE_INTRINSIC_FUNCTION(env, "omp_get_wtime", F64, omp_get_wtime)
{
    OMP_FUNC("omp_get_wtime");

    faabric::util::Clock& clock = faabric::util::getGlobalClock();
    long millis = clock.epochMillis();

    return ((F64)millis) / 1000;
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
    OMP_FUNC_ARGS("__kmpc_barrier {} {}", loc, globalTid);

    if (level->numThreads > 1) {
        level->barrier.wait();
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
    OMP_FUNC_ARGS("__kmpc_critical {} {} {}", loc, globalTid, crit);

    if (level->numThreads > 1) {
        level->levelMutex.lock();
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
    OMP_FUNC_ARGS("__kmpc_end_critical {} {} {}", loc, globalTid, crit);

    if (level->numThreads > 1) {
        level->levelMutex.unlock();
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
    OMP_FUNC_ARGS("__kmpc_flush{}", loc);

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
    OMP_FUNC_ARGS("__kmpc_master {} {}", loc, globalTid);

    return (I32)msg->appindex() == 0;
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
    OMP_FUNC_ARGS("__kmpc_end_master {} {}", loc, globalTid);

    if (msg->appindex() != 0) {
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
    OMP_FUNC_ARGS("__kmpc_single {} {}", loc, globalTid);

    return (I32)msg->appindex() == 0;
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
    OMP_FUNC_ARGS("__kmpc_end_single {} {}", loc, globalTid);

    if (msg->appindex() != 0) {
        throw std::runtime_error("Calling _kmpc_end_single from non-master");
    }
}

// ----------------------------------------------------
// FORKING
// ----------------------------------------------------

/**
 * The "real" version of this function is implemented in the openmp source at:
 * https://github.com/llvm/llvm-project/blob/main/openmp/runtime/src/kmp_csupport.cpp
 *
 * It calls into __kmp_fork call to do most of the work, which is here:
 * https://github.com/llvm/llvm-project/blob/main/openmp/runtime/src/kmp_runtime.cpp
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
    OMP_FUNC_ARGS(
      "__kmpc_fork_call {} {} {} {}", locPtr, argc, microtaskPtr, argsPtr);

    auto& sch = faabric::scheduler::getScheduler();

    WAVMWasmModule* parentModule = getExecutingWAVMModule();
    Runtime::Memory* memoryPtr = parentModule->defaultMemory;
    faabric::Message* parentCall = getExecutingCall();

    // Set up number of threads for next level
    std::shared_ptr<threads::Level> parentLevel = level;

    // Set up the next level
    auto nextLevel =
      std::make_shared<threads::Level>(parentLevel->getMaxThreadsAtNextLevel());
    nextLevel->fromParentLevel(parentLevel);

    // Check if we're only doing single-threaded
    bool isSingleThread = nextLevel->numThreads == 1;

    // Take memory snapshot
    std::string snapshotKey;
    if (!isSingleThread) {
        snapshotKey = parentModule->snapshot(false);
        logger->debug("Created OpenMP snapshot: {}", snapshotKey);
    } else {
        logger->debug("Not creating OpenMP snapshot for single thread");
    }

    const faabric::Message* originalCall = getExecutingCall();
    const std::string origStr =
      faabric::util::funcToString(*originalCall, false);

    // Prepare arguments for main thread and all others
    std::vector<IR::UntaggedValue> mainArguments = { 0, argc };
    std::vector<uint32_t> sharedVarPtrs;
    if (argc > 0) {
        // Build list of poitners to shared variables
        U32* sharedVarsPtr =
          Runtime::memoryArrayPtr<U32>(memoryPtr, argsPtr, argc);
        sharedVarPtrs =
          std::vector<uint32_t>(sharedVarsPtr, sharedVarsPtr + argc);

        // Append to main arguments
        mainArguments.insert(
          mainArguments.end(), sharedVarPtrs.begin(), sharedVarPtrs.end());
    }

    // Set up the chained calls
    // Note that the main OpenMP thread is always executed in this thread, hence
    // it is not scheduled remotely
    std::shared_ptr<faabric::Message> masterMsg = nullptr;
    std::vector<std::shared_ptr<faabric::Message>> childThreadMsgs;
    std::vector<int> remoteCallIds;
    for (int threadNum = 0; threadNum < nextLevel->numThreads; threadNum++) {
        // Create basic call
        std::shared_ptr<faabric::Message> call =
          faabric::util::messageFactoryShared(originalCall->user(),
                                              originalCall->function());

        // Record master message if necessary
        if (threadNum == 0) {
            masterMsg = call;
        }

        // All calls are async by definition
        call->set_isasync(true);

        // Snapshot details
        call->set_snapshotkey(snapshotKey);

        // Function pointer
        call->set_funcptr(microtaskPtr);

        // OpenMP thread number
        call->set_appindex(threadNum);

        if (threadNum > 0) {
            childThreadMsgs.push_back(call);
        }
    }

    // Attempt to schedule across hosts
    std::vector<std::future<int32_t>> localFutures;
    if (!isSingleThread) {
        // Set up the request
        faabric::BatchExecuteRequest req =
          faabric::util::batchExecFactory(childThreadMsgs);

        // Add remote context
        threads::SerialisedLevel serialisedLevel = nextLevel->serialise();
        req.set_contextdata(BYTES(&serialisedLevel),
                            threads::sizeOfSerialisedLevel(serialisedLevel));

        req.set_type(faabric::BatchExecuteRequest::THREADS);

        // Submit it
        std::vector<std::string> executedHosts = sch.callFunctions(req);

        // Iterate through messages and see which need to be executed in local
        // threads
        for (int i = 0; i < executedHosts.size(); i++) {
            std::string host = executedHosts.at(i);
            bool isLocal = host.empty();

            std::shared_ptr<faabric::Message> msg = childThreadMsgs.at(i);
            uint32_t msgId = msg->id();

            if (isLocal) {
                logger->debug("Dispatching local task for OpenMP call ID {}",
                              msgId);

                // Execute the local thread
                threads::OpenMPTask t(parentCall, msg, nextLevel);
                localFutures.emplace_back(parentModule->executeOpenMPTask(t));
            } else {
                // Function is being executed remotely
                logger->debug("Waiting for remote thread for call ID {}",
                              msgId);
                remoteCallIds.push_back(msgId);
                continue;
            }
        }
    }

    // Execute the master task (just invoke the microtask directly)
    IR::UntaggedValue mainThreadResult;
    {
        // We have to set up the context for the thread
        threads::setCurrentOpenMPLevel(nextLevel);
        setExecutingCall(masterMsg);

        // Execute the task
        logger->debug("OpenMP 0: executing OMP thread 0 (master)");
        WAVM::Runtime::Function* microtaskFunc =
          parentModule->getFunctionFromPtr(microtaskPtr);
        parentModule->executeFunction(
          microtaskFunc, mainArguments, mainThreadResult);

        // Now we reset the context for this main thread
        threads::setCurrentOpenMPLevel(parentLevel);
        setExecutingCall(parentMsg);
    }

    // Await the results of all the remote threads
    int numErrors = 0;
    for (auto callId : remoteCallIds) {
        int callTimeoutMs = faabric::util::getSystemConfig().chainedCallTimeout;

        logger->info(
          "Waiting for call id {} with a timeout of {}", callId, callTimeoutMs);

        int returnCode = 1;
        try {
            const faabric::Message result =
              sch.getFunctionResult(callId, callTimeoutMs);

            returnCode = result.returnvalue();

        } catch (faabric::redis::RedisNoResponseException& ex) {
            logger->error("Timed out waiting for chained call: {}", callId);

        } catch (std::exception& ex) {
            logger->error("Non-timeout exception waiting for chained call: {}",
                          ex.what());
        }

        if (returnCode > 0) {
            numErrors++;
        }
    }

    // Await local threads
    for (auto& f : localFutures) {
        int32_t retValue = f.get();
        if (retValue > 0) {
            throw std::runtime_error("OpenMP threads have errors");
        }
    }

    if (mainThreadResult.i32 > 0) {
        throw std::runtime_error("Master OpenMP thread failed");
    }

    // Reset parent level for next setting of threads
    parentLevel->pushedThreads = -1;

    // Delete the snapshot from registered hosts
    if (!isSingleThread) {
        PROF_START(BroadcastDeleteSnapshot)
        sch.broadcastSnapshotDelete(*originalCall, snapshotKey);
        PROF_END(BroadcastDeleteSnapshot)

        // Delete the snapshot locally
        PROF_START(DeleteSnapshot)
        faabric::snapshot::SnapshotRegistry& reg =
          faabric::snapshot::getSnapshotRegistry();
        reg.deleteSnapshot(snapshotKey);
        PROF_END(DeleteSnapshot)
    }
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
    std::shared_ptr<threads::Level> level = threads::getCurrentOpenMPLevel();

    if (level->numThreads == 1) {
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

            *stride = span * level->numThreads;
            *lower = *lower + (span * msg->appindex());
            *upper = *lower + span - incr;

            *lastIter =
              (msg->appindex() ==
               ((tripCount - 1) / (unsigned int)chunk) % level->numThreads);

            break;
        }

        case sch_static: { // (chunk not given)

            // If we have fewer trip_counts than threads
            if (tripCount < level->numThreads) {
                // Warning for future use, not tested at scale
                logger->warn("Small for loop trip count {} {}",
                             tripCount,
                             level->numThreads);

                if (msg->appindex() < tripCount) {
                    *upper = *lower = *lower + msg->appindex() * incr;
                } else {
                    *lower = *upper + incr;
                }

                *lastIter = (msg->appindex() == tripCount - 1);

            } else {
                // TODO: We only implement below kmp_sch_static_balanced, not
                // kmp_sch_static_greedy Those are set through KMP_SCHEDULE so
                // we would need to look out for real code setting this
                logger->debug("Ignores KMP_SCHEDULE variable, defaults to "
                              "static balanced schedule");

                U32 small_chunk = tripCount / level->numThreads;
                U32 extras = tripCount % level->numThreads;

                *lower += incr * (msg->appindex() * small_chunk +
                                  (msg->appindex() < extras ? msg->appindex()
                                                            : extras));

                *upper = *lower + small_chunk * incr -
                         (msg->appindex() < extras ? 0 : incr);

                *lastIter = (msg->appindex() == level->numThreads - 1);
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
    OMP_FUNC_ARGS("__kmpc_for_static_init_4 {} {} {} {} {} {} {} {} {}",
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
    OMP_FUNC_ARGS("__kmpc_for_static_init_4 {} {} {} {} {} {} {} {} {}",
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
    OMP_FUNC_ARGS("__kmpc_for_static_fini {} {}", loc, gtid);
}

// ---------------------------------------------------
// REDUCTION
// ---------------------------------------------------

int reduceFinished()
{
    std::shared_ptr<threads::Level> level = threads::getCurrentOpenMPLevel();

    // Notify the master thread that we've done our reduction
    if (msg->appindex() != 0) {
        level->masterWait(msg->appindex());
    }

    return 1;
}

/**
 *  Called to finish off a reduction.
 *  In a nowait scenario, just the master calls this function (the other threads
 *  have continued on).
 */
void finaliseReduce(bool barrier)
{
    std::shared_ptr<Level> level = getCurrentOpenMPLevel();
    faabric::Message* msg = getExecutingCall();

    // Master must make sure all other threads are done
    if (msg->appindex() == 0) {
        level->masterWait(0);
    }

    // Everyone waits if there's a barrier
    if (barrier) {
        PROF_START(FinaliseReduceBarrier)
        level->barrier.wait();
        PROF_END(FinaliseReduceBarrier)
    }
}

/**
 * It seems that in our case, always returning 1 for both kmpc_reduce and
 * kmpc_reduce_nowait gets the right result.
 *
 * In the OpenMP source we can see a more varied set of return values, but these
 * are for cases we don't yet support:
 * https://github.com/llvm/llvm-project/blob/main/openmp/runtime/src/kmp_csupport.cpp
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
                               I32 lockPtr)
{
    OMP_FUNC_ARGS("__kmpc_reduce {} {} {} {} {} {} {}",
                  loc,
                  gtid,
                  numVars,
                  reduceSize,
                  reduceData,
                  reduceFunc,
                  lockPtr);

    return reduceFinished();
}

/**
 * See __kmpc_reduce
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
                               I32 lockPtr)
{
    OMP_FUNC_ARGS("__kmpc_reduce_nowait {} {} {} {} {} {} {}",
                  loc,
                  gtid,
                  numVars,
                  reduceSize,
                  reduceData,
                  reduceFunc,
                  lockPtr);

    return reduceFinished();
}

/**
 * Finalises a blocking reduce, called by all threads.
 */
WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "__kmpc_end_reduce",
                               void,
                               __kmpc_end_reduce,
                               I32 loc,
                               I32 gtid,
                               I32 lck)
{
    OMP_FUNC_ARGS("__kmpc_end_reduce {} {} {}", loc, gtid, lck);
    finaliseReduce(true);
}

/**
 * Finalises a non-blocking reduce, called only by the master.
 */
WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "__kmpc_end_reduce_nowait",
                               void,
                               __kmpc_end_reduce_nowait,
                               I32 loc,
                               I32 gtid,
                               I32 lck)
{
    OMP_FUNC_ARGS("__kmpc_end_reduce_nowait {} {} {}", loc, gtid, lck);
    finaliseReduce(false);
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
    OMP_FUNC("omp_get_num_devices");
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
    OMP_FUNC_ARGS("omp_set_default_device {} (ignored)", defaultDeviceNumber);
}

// ----------------------------------------------
// ATOMICS
// ----------------------------------------------

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "__atomic_load",
                               void,
                               __atomic_load,
                               I32 a,
                               I32 b,
                               I32 c,
                               I32 d)
{
    OMP_FUNC_ARGS("__atomic_load {} {} {} {}", a, b, c, d);
    throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "__atomic_compare_exchange",
                               I32,
                               ___atomic_compare_exchange,
                               I32 a,
                               I32 b,
                               I32 c,
                               I32 d,
                               I32 e,
                               I32 f)
{
    OMP_FUNC_ARGS("__atomic_load {} {} {} {} {} {}", a, b, c, d, e, f);
    throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
}

void ompLink() {}
}
