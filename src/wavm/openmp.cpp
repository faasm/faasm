#include <faabric/proto/faabric.pb.h>
#include <faabric/scheduler/ExecutorContext.h>
#include <faabric/scheduler/Scheduler.h>
#include <faabric/snapshot/SnapshotRegistry.h>
#include <faabric/state/StateKeyValue.h>
#include <faabric/transport/PointToPointBroker.h>
#include <faabric/util/batch.h>
#include <faabric/util/func.h>
#include <faabric/util/gids.h>
#include <faabric/util/locks.h>
#include <faabric/util/logging.h>
#include <faabric/util/macros.h>
#include <faabric/util/memory.h>
#include <faabric/util/snapshot.h>
#include <faabric/util/string_tools.h>
#include <faabric/util/testing.h>
#include <faabric/util/timing.h>
#include <threads/ThreadState.h>
#include <wasm/WasmExecutionContext.h>
#include <wasm/WasmModule.h>
#include <wavm/WAVMWasmModule.h>

#include <WAVM/Platform/Thread.h>
#include <WAVM/Runtime/Intrinsics.h>
#include <WAVM/Runtime/Runtime.h>

using namespace WAVM;
using namespace faabric::scheduler;

namespace wasm {

// ------------------------------------------------
// LOGGING
// ------------------------------------------------

#define OMP_FUNC(str)                                                          \
    std::shared_ptr<threads::Level> level = threads::getCurrentOpenMPLevel();  \
    faabric::Message* msg = &ExecutorContext::get()->getMsg();                 \
    int localThreadNum = level->getLocalThreadNum(msg);                        \
    int globalThreadNum = level->getGlobalThreadNum(msg);                      \
    UNUSED(level);                                                             \
    UNUSED(msg);                                                               \
    UNUSED(localThreadNum);                                                    \
    UNUSED(globalThreadNum);                                                   \
    SPDLOG_TRACE("OMP {} ({}): " str, localThreadNum, globalThreadNum);

#define OMP_FUNC_ARGS(formatStr, ...)                                          \
    std::shared_ptr<threads::Level> level = threads::getCurrentOpenMPLevel();  \
    faabric::Message* msg = &ExecutorContext::get()->getMsg();                 \
    int localThreadNum = level->getLocalThreadNum(msg);                        \
    int globalThreadNum = level->getGlobalThreadNum(msg);                      \
    UNUSED(level);                                                             \
    UNUSED(msg);                                                               \
    UNUSED(localThreadNum);                                                    \
    UNUSED(globalThreadNum);                                                   \
    SPDLOG_TRACE("OMP {} ({}): " formatStr,                                    \
                 localThreadNum,                                               \
                 globalThreadNum,                                              \
                 __VA_ARGS__);

static std::shared_ptr<faabric::transport::PointToPointGroup>
getExecutingPointToPointGroup()
{
    faabric::Message& msg = ExecutorContext::get()->getMsg();
    return faabric::transport::PointToPointGroup::getOrAwaitGroup(
      msg.groupid());
}

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
    return localThreadNum;
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

    if (maxLevels < 0) {
        SPDLOG_WARN("Trying to set active level with a negative number {}",
                    maxLevels);
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

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "__kmpc_global_thread_num",
                               I32,
                               __kmpc_global_thread_num,
                               I32 loc)
{
    OMP_FUNC_ARGS("__kmpc_global_thread_num {}", loc);
    return globalThreadNum;
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
    getExecutingPointToPointGroup()->barrier(msg->groupidx());
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
        getExecutingPointToPointGroup()->lock(msg->groupidx(), true);

        // NOTE: here we need to pull the latest snapshot diffs from master.
        // This is a really inefficient way to implement a critical, and needs
        // more thought as to whether we can avoid doing a request/ response
        // every time.
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
        getExecutingPointToPointGroup()->unlock(msg->groupidx(), true);
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
    OMP_FUNC_ARGS("__kmpc_flush {}", loc);

    // Full memory fence, a bit overkill maybe for Wasm
    __sync_synchronize();
}

// ----------------------------------------------------
// MASTER
// ----------------------------------------------------

/**
 * Note: we only ensure the master section is run once, but do not handle
 * assigning to the master section.
 */
WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "__kmpc_master",
                               I32,
                               __kmpc_master,
                               I32 loc,
                               I32 globalTid)
{
    OMP_FUNC_ARGS("__kmpc_master {} {}", loc, globalTid);

    return localThreadNum == 0;
}

/**
 * Only called by the thread executing the master region.
 */
WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "__kmpc_end_master",
                               void,
                               __kmpc_end_master,
                               I32 loc,
                               I32 globalTid)
{
    OMP_FUNC_ARGS("__kmpc_end_master {} {}", loc, globalTid);

    if (localThreadNum != 0) {
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

    return localThreadNum == 0;
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

    if (localThreadNum != 0) {
        throw std::runtime_error("Calling _kmpc_end_single from non-master");
    }
}

// ----------------------------------------------------
// FORKING
// ----------------------------------------------------

/**
 * The LLVM version of this function is implemented in the openmp source at:
 * https://github.com/llvm/llvm-project/blob/main/openmp/runtime/src/kmp_csupport.cpp
 *
 * It calls into __kmp_fork call to do most of the work, which is here:
 * https://github.com/llvm/llvm-project/blob/main/openmp/runtime/src/kmp_runtime.cpp
 *
 * The structs passed in are defined in this file:
 * https://github.com/llvm/llvm-project/blob/main/openmp/runtime/src/kmp.h
 *
 * Arguments:
 * - locPtr = pointer to the source location info (type ident_t)
 * - nSharedVars = number of non-global shared variables
 * - microtaskPtr = function pointer for the microtask itself (microtask_t)
 * - sharedVarPtrs = pointer to an array of pointers to the non-global shared
 *   variables
 *
 * NOTE: the non-global shared variables include:
 * - those listed in a shared() directive
 * - those listed in a reduce() directive
 */

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "__kmpc_fork_call",
                               void,
                               __kmpc_fork_call,
                               I32 locPtr,
                               I32 nSharedVars,
                               I32 microtaskPtr,
                               I32 sharedVarPtrs)
{
    OMP_FUNC_ARGS("__kmpc_fork_call {} {} {} {}",
                  locPtr,
                  nSharedVars,
                  microtaskPtr,
                  sharedVarPtrs);

    WAVMWasmModule* parentModule = getExecutingWAVMModule();
    Runtime::Memory* memoryPtr = parentModule->defaultMemory;
    faabric::Message* parentCall = &ExecutorContext::get()->getMsg();

    const std::string parentStr =
      faabric::util::funcToString(*parentCall, false);

    // Set up the next level
    std::shared_ptr<threads::Level> parentLevel = level;
    auto nextLevel =
      std::make_shared<threads::Level>(parentLevel->getMaxThreadsAtNextLevel());
    nextLevel->fromParentLevel(parentLevel);

    // Set up shared variables
    if (nSharedVars > 0) {
        uint32_t* sharedVarsPtr = Runtime::memoryArrayPtr<uint32_t>(
          memoryPtr, sharedVarPtrs, nSharedVars);
        nextLevel->setSharedVarOffsets(sharedVarsPtr, nSharedVars);
    }

    if (nextLevel->depth > 1) {
        SPDLOG_ERROR("Nested OpenMP support removed");
        throw std::runtime_error("Nested OpenMP support removed");
    }

    // Set up the chained calls
    std::shared_ptr<faabric::BatchExecuteRequest> req =
      faabric::util::batchExecFactory(
        parentCall->user(), parentCall->function(), nextLevel->numThreads);
    req->set_type(faabric::BatchExecuteRequest::THREADS);
    req->set_subtype(ThreadRequestType::OPENMP);
    // TODO(thread-opt): we don't relate the calling message with the callee.
    // This means that OpenMP messages could be sub-optimally scheduled
    // faabric::util::updateBatchExecAppId(req, parentCall->appid());

    // In the local tests, we always set the single-host flag to avoid
    // having to synchronise snapshots
    if (faabric::util::isTestMode()) {
        req->set_singlehost(true);
    }

    // Add remote context
    std::vector<uint8_t> serialisedLevel = nextLevel->serialise();
    req->set_contextdata(serialisedLevel.data(), serialisedLevel.size());

    // Configure the mesages
    for (int i = 0; i < req->messages_size(); i++) {
        faabric::Message& m = req->mutable_messages()->at(i);

        // Function pointer
        m.set_funcptr(microtaskPtr);

        // OpenMP thread number
        int threadNum = nextLevel->getGlobalThreadNum(i);
        m.set_appidx(threadNum);

        // Group setup for distributed coordination. Note that the group index
        // is just within this function group, and not the global OpenMP
        // thread number
        m.set_groupidx(i);
    }

    // Execute the threads
    faabric::scheduler::Executor* executor =
      faabric::scheduler::ExecutorContext::get()->getExecutor();
    std::vector<std::pair<uint32_t, int>> results =
      executor->executeThreads(req, parentModule->getMergeRegions());

    for (auto [mid, res] : results) {
        if (res != 0) {
            SPDLOG_ERROR(
              "OpenMP thread failed, result {} on message {}", res, mid);
            throw std::runtime_error("OpenMP threads failed");
        }
    }

    // Clear this module's merge regions
    parentModule->clearMergeRegions();

    // Reset parent level for next setting of threads
    parentLevel->pushedThreads = -1;
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

    faabric::Message* msg = &ExecutorContext::get()->getMsg();
    std::shared_ptr<threads::Level> level = threads::getCurrentOpenMPLevel();
    int localThreadNum = level->getLocalThreadNum(msg);

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
            *lower = *lower + (span * localThreadNum);
            *upper = *lower + span - incr;

            *lastIter =
              (localThreadNum ==
               ((tripCount - 1) / (unsigned int)chunk) % level->numThreads);

            break;
        }

        case sch_static: { // (chunk not given)
            // If we have fewer trip_counts than threads
            if (tripCount < level->numThreads) {
                // Warning for future use, not tested at scale
                SPDLOG_WARN("Small for loop trip count {} {}",
                            tripCount,
                            level->numThreads);

                if (localThreadNum < tripCount) {
                    *upper = *lower = *lower + localThreadNum * incr;
                } else {
                    *lower = *upper + incr;
                }

                *lastIter = (localThreadNum == tripCount - 1);

            } else {
                // TODO: We only implement below kmp_sch_static_balanced, not
                // kmp_sch_static_greedy Those are set through KMP_SCHEDULE so
                // we would need to look out for real code setting this
                U32 small_chunk = tripCount / level->numThreads;
                U32 extras = tripCount % level->numThreads;

                *lower +=
                  incr * (localThreadNum * small_chunk +
                          (localThreadNum < extras ? localThreadNum : extras));

                *upper = *lower + small_chunk * incr -
                         (localThreadNum < extras ? 0 : incr);

                *lastIter = (localThreadNum == level->numThreads - 1);
            }

            *stride = tripCount;
            break;
        }
        default: {
            SPDLOG_ERROR("Unimplemented OpenMP scheduler {}", schedule);
            throw std::runtime_error("Unimplemented OpenMP scheduler");
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

/**
 * Called to start a reduction.
 */
void startReduceCritical(faabric::Message* msg,
                         std::shared_ptr<threads::Level> level,
                         int32_t numReduceVars,
                         int32_t reduceVarPtrs,
                         int32_t reduceVarsSize)

{
    // This function synchronises updates to shared reduce variables.
    // Each host will have its own copy of these variables, which will be merged
    // at the end of the parallel section via Faasm shared memory.
    // This means we only need to synchronise local accesses here, so we only
    // need a local lock.
    SPDLOG_TRACE("Entering reduce critical section for group {}",
                 msg->groupid());

    std::shared_ptr<faabric::transport::PointToPointGroup> group =
      faabric::transport::PointToPointGroup::getOrAwaitGroup(msg->groupid());
    group->localLock();
}

/**
 * Called to finish off a reduction.
 */
void endReduceCritical(faabric::Message* msg, bool barrier)
{
    std::shared_ptr<threads::Level> level = threads::getCurrentOpenMPLevel();
    int localThreadNum = level->getLocalThreadNum(msg);

    // Unlock the critical section
    std::shared_ptr<faabric::transport::PointToPointGroup> group =
      faabric::transport::PointToPointGroup::getGroup(msg->groupid());
    group->localUnlock();

    // Master must make sure all other threads are done
    group->notify(localThreadNum);

    // Everyone waits if there's a barrier
    if (barrier) {
        PROF_START(FinaliseReduceBarrier)
        group->barrier(localThreadNum);
        PROF_END(FinaliseReduceBarrier)
    }
}

/**
 * This function is called to start the critical section required to perform the
 * reduction operation by each thread. It will then call __kmpc_end_reduce (and
 * its nowait equivalent), when it's finished.
 *
 * It seems that in our case, always returning 1 for both kmpc_reduce and
 * kmpc_reduce_nowait gets the right result.
 *
 * In the OpenMP source we can see a more varied set of return values, but these
 * are for cases we don't yet support (notably teams):
 * https://github.com/llvm/llvm-project/blob/main/openmp/runtime/src/kmp_csupport.cpp
 *
 * Note that the reduce vars passed into this function are the *LOCAL* copies
 * on the thread's own stack used to hold intermediate results. There is
 * apparently no way to get a reference to the final destination of the
 * reduction result in this function, that is only known in kmpc_fork_call.
 */
WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "__kmpc_reduce",
                               I32,
                               __kmpc_reduce,
                               I32 loc,
                               I32 gtid,
                               I32 numReduceVars,
                               I32 reduceVarsSize,
                               I32 reduceVarPtrs,
                               I32 reduceFunc,
                               I32 lockPtr)
{
    OMP_FUNC_ARGS("__kmpc_reduce {} {} {} {} {} {} {}",
                  loc,
                  gtid,
                  numReduceVars,
                  reduceVarsSize,
                  reduceVarPtrs,
                  reduceFunc,
                  lockPtr);

    startReduceCritical(
      msg, level, numReduceVars, reduceVarPtrs, reduceVarsSize);
    return 1;
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
                               I32 numReduceVars,
                               I32 reduceVarsSize,
                               I32 reduceVarPtrs,
                               I32 reduceFunc,
                               I32 lockPtr)
{
    OMP_FUNC_ARGS("__kmpc_reduce_nowait {} {} {} {} {} {} {}",
                  loc,
                  gtid,
                  numReduceVars,
                  reduceVarsSize,
                  reduceVarPtrs,
                  reduceFunc,
                  lockPtr);

    startReduceCritical(
      msg, level, numReduceVars, reduceVarPtrs, reduceVarsSize);
    return 1;
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
    endReduceCritical(msg, true);
}

/**
 * Finalises a non-blocking reduce, called by all threads
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
    endReduceCritical(msg, false);
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
