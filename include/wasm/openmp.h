#pragma once

#include <faabric/executor/ExecutorContext.h>
#include <faabric/util/logging.h>
#include <faabric/util/macros.h>
#include <threads/ThreadState.h>

// ------------------------------------------------
// LOGGING
// ------------------------------------------------

#define OMP_FUNC(str)                                                          \
    std::shared_ptr<threads::Level> level = threads::getCurrentOpenMPLevel();  \
    faabric::Message* msg =                                                    \
      &faabric::executor::ExecutorContext::get()->getMsg();                    \
    int32_t localThreadNum = level->getLocalThreadNum(msg);                    \
    int32_t globalThreadNum = level->getGlobalThreadNum(msg);                  \
    UNUSED(level);                                                             \
    UNUSED(msg);                                                               \
    UNUSED(localThreadNum);                                                    \
    UNUSED(globalThreadNum);                                                   \
    SPDLOG_DEBUG("OMP {} ({}): " str, localThreadNum, globalThreadNum);

#define OMP_FUNC_ARGS(formatStr, ...)                                          \
    std::shared_ptr<threads::Level> level = threads::getCurrentOpenMPLevel();  \
    faabric::Message* msg =                                                    \
      &faabric::executor::ExecutorContext::get()->getMsg();                    \
    int32_t localThreadNum = level->getLocalThreadNum(msg);                    \
    int32_t globalThreadNum = level->getGlobalThreadNum(msg);                  \
    UNUSED(level);                                                             \
    UNUSED(msg);                                                               \
    UNUSED(localThreadNum);                                                    \
    UNUSED(globalThreadNum);                                                   \
    SPDLOG_DEBUG("OMP {} ({}): " formatStr,                                    \
                 localThreadNum,                                               \
                 globalThreadNum,                                              \
                 __VA_ARGS__);

namespace wasm {
void doOpenMPBarrier(int32_t loc, int32_t globalTid);

void doOpenMPCritical(int32_t loc, int32_t globalTid, int32_t crit);

void doOpenMPEndCritical(int32_t loc, int32_t globalTid, int32_t crit);

void doOpenMPFork(int32_t loc,
                  int32_t nSharedVars,
                  int32_t microTask,
                  uint32_t* sharedVars);

void doOpenMPForStaticInit4(int32_t loc,
                            int32_t gtid,
                            int32_t schedule,
                            int32_t* lastIter,
                            int32_t* lower,
                            int32_t* upper,
                            int32_t* stride,
                            int32_t incr,
                            int32_t chunk);

void doOpenMPForStaticInit8(int32_t loc,
                            int32_t gtid,
                            int32_t schedule,
                            int32_t* lastIter,
                            int64_t* lower,
                            int64_t* upper,
                            int64_t* stride,
                            int32_t incr,
                            int32_t chunk);

void doOpenMPForStaticFini(int32_t loc, int32_t globalTid);

int32_t doOpenMPGetMaxThreads();

int32_t doOpenMPGetNumThreads();

int32_t doOpenMPGetThreadNum();

double doOpenMPGetWTime();

int32_t doOpenMPGlobalThreadNum(int32_t loc);

int32_t doOpenMPMaster(int32_t loc, int32_t globalTid);

void doOpenMPEndMaster(int32_t loc, int32_t globalTid);

void doOpenMPPushNumThreads(int32_t loc, int32_t globalTid, int32_t numThreads);

void doOpenMPSetNumThreads(int32_t numThreads);

int32_t doOpenMPSingle(int32_t loc, int32_t globalTid);

void doOpenMPEndSingle(int32_t loc, int32_t globalTid);

void doOpenMPStartReduceCritical(faabric::Message* msg,
                                 std::shared_ptr<threads::Level> level,
                                 int32_t numReduceVars,
                                 int32_t reduceVarPtrs,
                                 int32_t reduceVarsSize);

void doOpenMPEndReduceCritical(faabric::Message* msg, bool barrier);
}
