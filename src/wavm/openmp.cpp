#include <faabric/batch-scheduler/SchedulingDecision.h>
#include <faabric/executor/ExecutorContext.h>
#include <faabric/proto/faabric.pb.h>
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
#include <wasm/openmp.h>
#include <wavm/WAVMWasmModule.h>

#include <WAVM/Platform/Thread.h>
#include <WAVM/Runtime/Intrinsics.h>
#include <WAVM/Runtime/Runtime.h>

using namespace WAVM;
using namespace faabric::executor;

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
    return wasm::doOpenMPGetThreadNum();
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
    return wasm::doOpenMPGetNumThreads();
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
    wasm::doOpenMPPushNumThreads(loc, globalTid, numThreads);
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "omp_set_num_threads",
                               void,
                               omp_set_num_threads,
                               I32 numThreads)
{
    wasm::doOpenMPSetNumThreads(numThreads);
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "__kmpc_global_thread_num",
                               I32,
                               __kmpc_global_thread_num,
                               I32 loc)
{
    return wasm::doOpenMPGlobalThreadNum(loc);
}

// ------------------------------------------------
// TIMING
// ------------------------------------------------

WAVM_DEFINE_INTRINSIC_FUNCTION(env, "omp_get_wtime", F64, omp_get_wtime)
{
    return wasm::doOpenMPGetWTime();
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "__kmpc_barrier",
                               void,
                               __kmpc_barrier,
                               I32 loc,
                               I32 globalTid)
{
    wasm::doOpenMPBarrier(loc, globalTid);
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "__kmpc_critical",
                               void,
                               __kmpc_critical,
                               I32 loc,
                               I32 globalTid,
                               I32 crit)
{
    wasm::doOpenMPCritical(loc, globalTid, crit);
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
    wasm::doOpenMPEndCritical(loc, globalTid, crit);
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

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "__kmpc_master",
                               I32,
                               __kmpc_master,
                               I32 loc,
                               I32 globalTid)
{
    return wasm::doOpenMPMaster(loc, globalTid);
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
    wasm::doOpenMPEndMaster(loc, globalTid);
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

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "__kmpc_fork_call",
                               void,
                               __kmpc_fork_call,
                               I32 locPtr,
                               I32 nSharedVars,
                               I32 microtaskPtr,
                               I32 sharedVarPtrs)
{
    WAVMWasmModule* parentModule = getExecutingWAVMModule();
    Runtime::Memory* memoryPtr = parentModule->defaultMemory;

    // Set up shared variables
    uint32_t* sharedVarsPtr = nullptr;
    if (nSharedVars > 0) {
        sharedVarsPtr = Runtime::memoryArrayPtr<uint32_t>(
          memoryPtr, sharedVarPtrs, nSharedVars);
    }

    wasm::doOpenMPFork(locPtr, nSharedVars, microtaskPtr, sharedVarsPtr);
}

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
    // Get host pointers for the things we need to write
    Runtime::Memory* memoryPtr = getExecutingWAVMModule()->defaultMemory;

    I32* lastIter = &Runtime::memoryRef<I32>(memoryPtr, lastIterPtr);
    I32* lower = &Runtime::memoryRef<I32>(memoryPtr, lowerPtr);
    I32* upper = &Runtime::memoryRef<I32>(memoryPtr, upperPtr);
    I32* stride = &Runtime::memoryRef<I32>(memoryPtr, stridePtr);

    wasm::doOpenMPForStaticInit4(
      loc, gtid, schedule, lastIter, lower, upper, stride, incr, chunk);
}

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
    // Get host pointers for the things we need to write
    Runtime::Memory* memoryPtr = getExecutingWAVMModule()->defaultMemory;
    I32* lastIter = &Runtime::memoryRef<I32>(memoryPtr, lastIterPtr);
    I64* lower = &Runtime::memoryRef<I64>(memoryPtr, lowerPtr);
    I64* upper = &Runtime::memoryRef<I64>(memoryPtr, upperPtr);
    I64* stride = &Runtime::memoryRef<I64>(memoryPtr, stridePtr);

    wasm::doOpenMPForStaticInit8(
      loc, gtid, schedule, lastIter, lower, upper, stride, incr, chunk);
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "__kmpc_for_static_fini",
                               void,
                               __kmpc_for_static_fini,
                               I32 loc,
                               I32 gtid)
{
    wasm::doOpenMPForStaticFini(loc, gtid);
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
