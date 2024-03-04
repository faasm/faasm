#include <faabric/executor/ExecutorContext.h>
#include <faabric/planner/PlannerClient.h>
#include <faabric/transport/PointToPointBroker.h>
#include <faabric/util/batch.h>
#include <faabric/util/func.h>
#include <faabric/util/testing.h>
#include <wasm/WasmExecutionContext.h>
#include <wasm/WasmModule.h>
#include <wasm/openmp.h>

namespace wasm {
static std::shared_ptr<faabric::transport::PointToPointGroup>
getExecutingPointToPointGroup()
{
    faabric::Message& msg = faabric::executor::ExecutorContext::get()->getMsg();
    return faabric::transport::PointToPointGroup::getOrAwaitGroup(
      msg.groupid());
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
void doOpenMPBarrier(int32_t loc, int32_t globalTid)
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
void doOpenMPCritical(int32_t loc, int32_t globalTid, int32_t crit)
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

void doOpenMPEndCritical(int32_t loc, int32_t globalTid, int32_t crit)
{
    OMP_FUNC_ARGS("__kmpc_end_critical {} {} {}", loc, globalTid, crit);

    if (level->numThreads > 1) {
        getExecutingPointToPointGroup()->unlock(msg->groupidx(), true);
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
void doOpenMPFork(int32_t loc,
                  int32_t nSharedVars,
                  int32_t microTask,
                  uint32_t* sharedVars)
{
    OMP_FUNC_ARGS("__kmpc_fork_call {} {} {}", loc, nSharedVars, microTask);

    // To replicate the fork behaviour, we create (n - 1) executors with thread
    // semantics (i.e. sharing the same Faaslet). And instruct the calling
    // (parent) executor to also execute the same micro task
    auto* parentCall = &faabric::executor::ExecutorContext::get()->getMsg();
    auto* parentModule = getExecutingModule();
    auto parentReq =
      faabric::executor::ExecutorContext::get()->getBatchRequest();
    const auto parentStr = faabric::util::funcToString(*parentCall, false);
    auto* parentExecutor =
      faabric::executor::ExecutorContext::get()->getExecutor();

    // OpenMP execution contexs are called levels, and they contain the
    // thread-local information to execute the microTask (mostly private and
    // shared variables)
    std::shared_ptr<threads::Level> parentLevel = level;
    auto nextLevel =
      std::make_shared<threads::Level>(parentLevel->getMaxThreadsAtNextLevel());
    nextLevel->fromParentLevel(parentLevel);

    // Set up shared variables
    if (nSharedVars > 0) {
        nextLevel->setSharedVarOffsets(sharedVars, nSharedVars);
    }

    if (nextLevel->depth > 1) {
        SPDLOG_ERROR("Nested OpenMP support removed");
        throw std::runtime_error("Nested OpenMP support removed");
    }

    // Set up the chained calls with thread semantics
    std::shared_ptr<faabric::BatchExecuteRequest> req =
      faabric::util::batchExecFactory(
        parentCall->user(), parentCall->function(), nextLevel->numThreads - 1);
    req->set_type(faabric::BatchExecuteRequest::THREADS);
    req->set_subtype(ThreadRequestType::OPENMP);
    // Propagate the app ID to let the planner know that these are messages
    // for the same app
    faabric::util::updateBatchExecAppId(req, parentCall->appid());
    // Propagate the single-host hint. The single host flag can be used to hint
    // that we do not need to preemptively distribute snapshots
    req->set_singlehosthint(parentReq->singlehosthint());
    // Serialise the level so that it is available in the request
    std::vector<uint8_t> serialisedLevel = nextLevel->serialise();
    req->set_contextdata(serialisedLevel.data(), serialisedLevel.size());

    // Configure the mesages
    for (int i = 0; i < req->messages_size(); i++) {
        faabric::Message& m = req->mutable_messages()->at(i);

        // Function pointer in the WASM sense (so just an integer to the
        // function table)
        m.set_funcptr(microTask);

        // OpenMP thread number
        int threadNum = nextLevel->getGlobalThreadNum(i + 1);
        m.set_appidx(threadNum);

        // Group setup for distributed coordination. Note that the group index
        // is just within this function group, and not the global OpenMP
        // thread number
        m.set_groupidx(i + 1);
    }

    // Do snapshotting if not on a single host. Note that, from the caller
    // thread, we cannot know if the request is going to be single host or not.
    // So, by default, we always take a snapshot. We can bypass this by setting
    // the single host hint flag in the caller request
    // TODO: ideally, we would first call the planner to know if the scheduling
    // decision will be single host or not, and then have the threads wait for
    // the snapshot if necessary (i.e. getOrAwaitSnapshot())
    std::shared_ptr<faabric::util::SnapshotData> snap = nullptr;
    if (!req->singlehosthint()) {
        snap = parentExecutor->getMainThreadSnapshot(*parentCall, true);

        // Get dirty regions since last batch of threads
        std::span<uint8_t> memView = parentExecutor->getMemoryView();
        faabric::util::getDirtyTracker()->stopTracking(memView);
        faabric::util::getDirtyTracker()->stopThreadLocalTracking(memView);

        // If this is the first batch, these dirty regions will be empty
        std::vector<char> dirtyRegions =
          faabric::util::getDirtyTracker()->getBothDirtyPages(memView);

        // Apply changes to snapshot
        snap->fillGapsWithBytewiseRegions();
        std::vector<faabric::util::SnapshotDiff> updates =
          snap->diffWithDirtyRegions(memView, dirtyRegions);

        if (updates.empty()) {
            SPDLOG_DEBUG(
              "No updates to main thread snapshot for {} over {} pages",
              parentStr,
              dirtyRegions.size());
        } else {
            SPDLOG_DEBUG("Updating main thread snapshot for {} with {} diffs",
                         parentStr,
                         updates.size());
            snap->applyDiffs(updates);
        }

        // Clear merge regions, not persisted between batches of threads
        snap->clearMergeRegions();

        // Now we have to add any merge regions we've been saving up for this
        // next batch of threads
        auto mergeRegions = parentModule->getMergeRegions();
        for (const auto& mr : mergeRegions) {
            snap->addMergeRegion(
              mr.offset, mr.length, mr.dataType, mr.operation);
        }
    }

    // Invoke all non-main threads
    faabric::batch_scheduler::SchedulingDecision decision(req->appid(), 0);
    if (req->messages_size() > 0) {
        decision = faabric::planner::getPlannerClient().callFunctions(req);
    } else {
        // In a one-thread OpenMP loop, we manually create a communication
        // group of size one
        const std::string thisHost =
          faabric::util::getSystemConfig().endpointHost;
        decision.addMessage(thisHost, parentCall->id(), 0, 0);
        faabric::transport::getPointToPointBroker()
          .setUpLocalMappingsFromSchedulingDecision(decision);
    }

    // Invoke the main thread (number zero)
    auto thisThreadReq = faabric::util::batchExecFactory(
      parentCall->user(), parentCall->function(), 1);
    thisThreadReq->set_type(faabric::BatchExecuteRequest::THREADS);
    thisThreadReq->set_subtype(ThreadRequestType::OPENMP);
    thisThreadReq->set_contextdata(serialisedLevel.data(),
                                   serialisedLevel.size());
    thisThreadReq->set_singlehost(parentReq->singlehost());
    thisThreadReq->set_singlehosthint(parentReq->singlehosthint());
    // Update the group and batch id for inter-thread communication
    faabric::util::updateBatchExecAppId(thisThreadReq, parentCall->appid());
    faabric::util::updateBatchExecGroupId(thisThreadReq, decision.groupId);
    auto& m = thisThreadReq->mutable_messages()->at(0);
    m.set_appidx(0);
    m.set_groupidx(0);
    m.set_funcptr(microTask);

    // Finally, set the executor context, execute, and reset the context
    faabric::executor::ExecutorContext::set(parentExecutor, thisThreadReq, 0);
    if (!decision.isSingleHost()) {
        faabric::util::getDirtyTracker()->startThreadLocalTracking(
          parentExecutor->getMemoryView());
    }
    auto returnValue = parentModule->executeTask(0, 0, thisThreadReq);
    faabric::executor::ExecutorContext::set(parentExecutor, parentReq, 0);

    // Process and set thread result
    if (returnValue != 0) {
        SPDLOG_ERROR("OpenMP thread (0) failed, result {} on message {}",
                     thisThreadReq->messages(0).returnvalue(),
                     thisThreadReq->messages(0).id());
        throw std::runtime_error("OpenMP threads failed");
    }

    // Wait for all other threads to finish
    for (int i = 0; i < req->messages_size(); i++) {
        uint32_t messageId = req->messages().at(i).id();

        auto msgResult = faabric::planner::getPlannerClient().getMessageResult(
          req->appid(),
          messageId,
          10 * faabric::util::getSystemConfig().boundTimeout);

        if (msgResult.returnvalue() != 0) {
            SPDLOG_ERROR("OpenMP thread ({}) failed, result {} on message {}",
                         i + 1,
                         msgResult.returnvalue(),
                         msgResult.id());
            throw std::runtime_error("OpenMP threads failed");
        }
    }

    // Perform snapshot updates if not on single host. Note that, here we know
    // for sure that we must do dirty tracking, and are the last thread in
    // the batch
    if (!decision.isSingleHost()) {
        // First, get all the thread local diffs for this thread
        std::span<uint8_t> memView = parentExecutor->getMemoryView();
        faabric::util::getDirtyTracker()->stopThreadLocalTracking(memView);
        auto thisThreadDirtyRegions =
          faabric::util::getDirtyTracker()->getThreadLocalDirtyPages(memView);

        // Second, get the diffs for the batch request that has executed locally
        auto diffs = parentExecutor->mergeDirtyRegions(*parentCall,
                                                       thisThreadDirtyRegions);
        snap->queueDiffs(diffs);

        // Write queued diffs (local and remote) to snapshot
        int nWritten = snap->writeQueuedDiffs();

        // Remap memory to snapshot if it's been updated
        if (nWritten > 0) {
            parentExecutor->setMemorySize(snap->getSize());
            snap->mapToMemory(memView);
        }

        // Start tracking again
        memView = parentExecutor->getMemoryView();
        faabric::util::getDirtyTracker()->startTracking(memView);
        faabric::util::getDirtyTracker()->startThreadLocalTracking(memView);
    }

    // Clear this module's merge regions
    parentModule->clearMergeRegions();

    // Reset parent level for next setting of threads
    threads::setCurrentOpenMPLevel(parentLevel);
    parentLevel->pushedThreads = -1;
}

// -------------------------------------------------------
// FOR LOOP STATIC INIT
// -------------------------------------------------------

enum sched_type : int32_t
{
    sch_lower = 32, /**< lower bound for unordered values */
    sch_static_chunked = 33,
    sch_static = 34, /**< static unspecialized */
};

template<typename T>
void for_static_init(int32_t schedule,
                     int32_t* lastIter,
                     T* lower,
                     T* upper,
                     T* stride,
                     T incr,
                     T chunk);

template<typename T>
void for_static_init(int32_t schedule,
                     int32_t* lastIter,
                     T* lower,
                     T* upper,
                     T* stride,
                     T incr,
                     T chunk)
{
    // Unsigned version of the given template parameter
    typedef typename std::make_unsigned<T>::type UT;

    faabric::Message* msg =
      &faabric::executor::ExecutorContext::get()->getMsg();
    std::shared_ptr<threads::Level> level = threads::getCurrentOpenMPLevel();
    int32_t localThreadNum = level->getLocalThreadNum(msg);

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
        tripCount = (int32_t)(*upper - *lower) / incr + 1;

    } else {
        tripCount = (int32_t)(*lower - *upper) / (-incr) + 1;
    }

    switch (schedule) {
        case sch_static_chunked: {
            int32_t span;

            if (chunk < 1) {
                chunk = 1;
            }

            span = chunk * incr;

            *stride = span * level->numThreads;
            *lower = *lower + (span * localThreadNum);
            *upper = *lower + span - incr;

            *lastIter = (localThreadNum == ((tripCount - 1) / (uint32_t)chunk) %
                                             level->numThreads);

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
                uint32_t small_chunk = tripCount / level->numThreads;
                uint32_t extras = tripCount % level->numThreads;

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
void doOpenMPForStaticInit4(int32_t loc,
                            int32_t gtid,
                            int32_t schedule,
                            int32_t* lastIter,
                            int32_t* lower,
                            int32_t* upper,
                            int32_t* stride,
                            int32_t incr,
                            int32_t chunk)
{
    OMP_FUNC_ARGS("__kmpc_for_static_init_4 {} {} {} {} {} {} {} {} {}",
                  loc,
                  gtid,
                  schedule,
                  *lastIter,
                  *lower,
                  *upper,
                  *stride,
                  incr,
                  chunk);

    for_static_init<int32_t>(
      schedule, lastIter, lower, upper, stride, incr, chunk);
}

/*
 * See __kmpc_for_static_init_4
 */
void doOpenMPForStaticInit8(int32_t loc,
                            int32_t gtid,
                            int32_t schedule,
                            int32_t* lastIter,
                            int64_t* lower,
                            int64_t* upper,
                            int64_t* stride,
                            int32_t incr,
                            int32_t chunk)
{
    OMP_FUNC_ARGS("__kmpc_for_static_init_8 {} {} {} {} {} {} {} {} {}",
                  loc,
                  gtid,
                  schedule,
                  *lastIter,
                  *lower,
                  *upper,
                  *stride,
                  incr,
                  chunk);

    for_static_init<int64_t>(
      schedule, lastIter, lower, upper, stride, incr, chunk);
}

void doOpenMPForStaticFini(int32_t loc, int32_t globalTid)
{
    OMP_FUNC_ARGS("__kmpc_for_static_fini {} {}", loc, globalTid);
}

int32_t doOpenMPGetNumThreads()
{
    OMP_FUNC("omp_get_num_threads")
    return level->numThreads;
}

int32_t doOpenMPGetThreadNum()
{
    OMP_FUNC("omp_get_thread_num")
    return localThreadNum;
}

double doOpenMPGetWTime()
{
    OMP_FUNC("omp_get_wtime");

    auto& clock = faabric::util::getGlobalClock();
    long millis = clock.epochMillis();

    return ((double)millis) / 1000;
}

int32_t doOpenMPGlobalThreadNum(int32_t loc)
{
    OMP_FUNC_ARGS("__kmpc_global_thread_num {}", loc);
    return globalThreadNum;
}

// ----------------------------------------------------
// MASTER
// ----------------------------------------------------

/**
 * Note: we only ensure the master section is run once, but do not handle
 * assigning to the master section.
 */
int32_t doOpenMPMaster(int32_t loc, int32_t globalTid)
{
    OMP_FUNC_ARGS("__kmpc_master {} {}", loc, globalTid);

    return localThreadNum == 0;
}

void doOpenMPPushNumThreads(int32_t loc, int32_t globalTid, int32_t numThreads)
{
    OMP_FUNC_ARGS(
      "__kmpc_push_num_threads {} {} {}", loc, globalTid, numThreads);

    if (numThreads > 0) {
        level->pushedThreads = numThreads;
    }
}

void doOpenMPEndMaster(int32_t loc, int32_t globalTid)
{
    OMP_FUNC_ARGS("__kmpc_end_master {} {}", loc, globalTid);

    if (localThreadNum != 0) {
        throw std::runtime_error("Calling _kmpc_end_master from non-master");
    }
}

void doOpenMPSetNumThreads(int32_t numThreads)
{
    OMP_FUNC_ARGS("omp_set_num_threads {}", numThreads);

    if (numThreads > 0) {
        level->wantedThreads = numThreads;
    }
}
}
