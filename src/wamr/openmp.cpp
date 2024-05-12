#include <wamr/WAMRWasmModule.h>
#include <wasm/openmp.h>

#include <wamr/native.h>
#include <wasm_export.h>

#define CALL_OPENMP_CATCH_EXCETION_NO_RETURN(call)                             \
    try {                                                                      \
        call;                                                                  \
    } catch (std::exception & e) {                                             \
        auto __module = wasm::getExecutingWAMRModule();                        \
        __module->doThrowException(e);                                         \
    }

namespace wasm {
static void __kmpc_barrier_wrapper(wasm_exec_env_t execEnv,
                                   int32_t loc,
                                   int32_t globalTid)
{
    CALL_OPENMP_CATCH_EXCETION_NO_RETURN(wasm::doOpenMPBarrier(loc, globalTid));
}

static void __kmpc_critical_wrapper(wasm_exec_env_t execEnv,
                                    int32_t loc,
                                    int32_t globalTid,
                                    int32_t crit)
{
    CALL_OPENMP_CATCH_EXCETION_NO_RETURN(
      wasm::doOpenMPCritical(loc, globalTid, crit));
}

static void __kmpc_end_critical_wrapper(wasm_exec_env_t execEnv,
                                        int32_t loc,
                                        int32_t globalTid,
                                        int32_t crit)
{
    CALL_OPENMP_CATCH_EXCETION_NO_RETURN(
      wasm::doOpenMPEndCritical(loc, globalTid, crit));
}

static void __kmpc_end_master_wrapper(wasm_exec_env_t execEnv,
                                      int32_t loc,
                                      int32_t globalTid)
{
    CALL_OPENMP_CATCH_EXCETION_NO_RETURN(
      wasm::doOpenMPEndMaster(loc, globalTid));
}

static void __kmpc_end_reduce_wrapper(wasm_exec_env_t execEnv,
                                      int32_t loc,
                                      int32_t gtid,
                                      int32_t lck)
{
    OMP_FUNC_ARGS("__kmpc_end_reduce {} {} {}", loc, gtid, lck);
    CALL_OPENMP_CATCH_EXCETION_NO_RETURN(
      wasm::doOpenMPEndReduceCritical(msg, true));
}

static void __kmpc_end_reduce_nowait_wrapper(wasm_exec_env_t execEnv,
                                             int32_t loc,
                                             int32_t gtid,
                                             int32_t lck)
{
    OMP_FUNC_ARGS("__kmpc_end_reduce_nowait {} {} {}", loc, gtid, lck);
    CALL_OPENMP_CATCH_EXCETION_NO_RETURN(
      wasm::doOpenMPEndReduceCritical(msg, false));
}

static void __kmpc_end_single_wrapper(wasm_exec_env_t execEnv,
                                      int32_t loc,
                                      int32_t globalTid)
{
    CALL_OPENMP_CATCH_EXCETION_NO_RETURN(
      wasm::doOpenMPEndSingle(loc, globalTid));
}

static void __kmpc_flush_wrapper(wasm_exec_env_t execEnv, int32_t loc)
{
    CALL_OPENMP_CATCH_EXCETION_NO_RETURN(wasm::doOpenMPFlush(loc));
}

static int32_t __kmpc_single_wrapper(wasm_exec_env_t execEnv,
                                     int32_t loc,
                                     int32_t globalTid)
{
    return wasm::doOpenMPSingle(loc, globalTid);
}

static void __kmpc_for_static_fini_wrapper(wasm_exec_env_t execEnv,
                                           int32_t loc,
                                           int32_t gtid)
{
    CALL_OPENMP_CATCH_EXCETION_NO_RETURN(
      wasm::doOpenMPForStaticFini(loc, gtid));
}

static void __kmpc_for_static_init_4_wrapper(wasm_exec_env_t execEnv,
                                             int32_t loc,
                                             int32_t gtid,
                                             int32_t schedule,
                                             int32_t* lastIter,
                                             int32_t* lower,
                                             int32_t* upper,
                                             int32_t* stride,
                                             int32_t incr,
                                             int32_t chunk)
{
    CALL_OPENMP_CATCH_EXCETION_NO_RETURN(wasm::doOpenMPForStaticInit4(
      loc, gtid, schedule, lastIter, lower, upper, stride, incr, chunk));
}

static void __kmpc_for_static_init_8_wrapper(wasm_exec_env_t execEnv,
                                             int32_t loc,
                                             int32_t gtid,
                                             int32_t schedule,
                                             int32_t* lastIter,
                                             int64_t* lower,
                                             int64_t* upper,
                                             int64_t* stride,
                                             int64_t incr,
                                             int64_t chunk)
{
    CALL_OPENMP_CATCH_EXCETION_NO_RETURN(wasm::doOpenMPForStaticInit8(
      loc, gtid, schedule, lastIter, lower, upper, stride, incr, chunk));
}

static void __kmpc_fork_call_wrapper(wasm_exec_env_t execEnv,
                                     int32_t locPtr,
                                     int32_t nSharedVars,
                                     int32_t microTaskPtr,
                                     uint32_t sharedVarsPtr)
{
    // Set-up shared variables
    auto* wamrModule = wasm::getExecutingWAMRModule();
    wamrModule->validateWasmOffset(sharedVarsPtr,
                                   nSharedVars * sizeof(int32_t));
    uint32_t* nativeSharedVarsPtr =
      (uint32_t*)wamrModule->wasmOffsetToNativePointer(sharedVarsPtr);

    // Create child thread's execution environments
    wamrModule->createThreadsExecEnv(execEnv);

    // Fork is complex enough that we try/catch different exceptions inside
    // to avoid missing relevant errors
    wasm::doOpenMPFork(locPtr, nSharedVars, microTaskPtr, nativeSharedVarsPtr);

    // Clean-up child execution enviroments
    wamrModule->destroyThreadsExecEnv();
}

static int32_t __kmpc_global_thread_num_wrapper(wasm_exec_env_t exec_env,
                                                int32_t loc)
{
    return wasm::doOpenMPGlobalThreadNum(loc);
}

static int32_t __kmpc_master_wrapper(wasm_exec_env_t exec_env,
                                     int32_t loc,
                                     int32_t globalTid)
{
    return wasm::doOpenMPMaster(loc, globalTid);
}

static void __kmpc_push_num_threads_wrapper(wasm_exec_env_t execEnv,
                                            int32_t loc,
                                            int32_t globalTid,
                                            int32_t numThreads)
{
    wasm::doOpenMPPushNumThreads(loc, globalTid, numThreads);
}

static int32_t __kmpc_reduce_wrapper(wasm_exec_env_t execEnv,
                                     int32_t loc,
                                     int32_t gtid,
                                     int32_t numReduceVars,
                                     int32_t reduceVarsSize,
                                     int32_t reduceVarPtrs,
                                     int32_t reduceFunc,
                                     int32_t lockPtr)
{
    OMP_FUNC_ARGS("__kmpc_reduce {} {} {} {} {} {} {}",
                  loc,
                  gtid,
                  numReduceVars,
                  reduceVarsSize,
                  reduceVarPtrs,
                  reduceFunc,
                  lockPtr);

    CALL_OPENMP_CATCH_EXCETION_NO_RETURN(wasm::doOpenMPStartReduceCritical(
      msg, level, numReduceVars, reduceVarPtrs, reduceVarsSize));

    return 1;
}

static int32_t __kmpc_reduce_nowait_wrapper(wasm_exec_env_t execEnv,
                                            int32_t loc,
                                            int32_t gtid,
                                            int32_t numReduceVars,
                                            int32_t reduceVarsSize,
                                            int32_t reduceVarPtrs,
                                            int32_t reduceFunc,
                                            int32_t lockPtr)
{
    OMP_FUNC_ARGS("__kmpc_reduce_nowait {} {} {} {} {} {} {}",
                  loc,
                  gtid,
                  numReduceVars,
                  reduceVarsSize,
                  reduceVarPtrs,
                  reduceFunc,
                  lockPtr);

    CALL_OPENMP_CATCH_EXCETION_NO_RETURN(wasm::doOpenMPStartReduceCritical(
      msg, level, numReduceVars, reduceVarPtrs, reduceVarsSize));

    return 1;
}

static int32_t omp_get_max_threads_wrapper(wasm_exec_env_t execEnv)
{
    return wasm::doOpenMPGetMaxThreads();
}

static int32_t omp_get_num_threads_wrapper(wasm_exec_env_t execEnv)
{
    return wasm::doOpenMPGetNumThreads();
}

static int32_t omp_get_thread_num_wrapper(wasm_exec_env_t execEnv)
{
    return wasm::doOpenMPGetThreadNum();
}

static double omp_get_wtime_wrapper(wasm_exec_env_t execEnv)
{
    return wasm::doOpenMPGetWTime();
}

static void omp_set_num_threads_wrapper(wasm_exec_env_t execEnv,
                                        int32_t numThreads)
{
    wasm::doOpenMPSetNumThreads(numThreads);
}

static NativeSymbol ns[] = {
    REG_NATIVE_FUNC(__kmpc_barrier, "(ii)"),
    REG_NATIVE_FUNC(__kmpc_critical, "(iii)"),
    REG_NATIVE_FUNC(__kmpc_end_critical, "(iii)"),
    REG_NATIVE_FUNC(__kmpc_end_master, "(ii)"),
    REG_NATIVE_FUNC(__kmpc_end_reduce, "(iii)"),
    REG_NATIVE_FUNC(__kmpc_end_reduce_nowait, "(iii)"),
    REG_NATIVE_FUNC(__kmpc_end_single, "(ii)"),
    REG_NATIVE_FUNC(__kmpc_flush, "(i)"),
    REG_NATIVE_FUNC(__kmpc_for_static_fini, "(ii)"),
    REG_NATIVE_FUNC(__kmpc_for_static_init_4, "(iii****ii)"),
    REG_NATIVE_FUNC(__kmpc_for_static_init_8, "(iii****II)"),
    REG_NATIVE_FUNC(__kmpc_fork_call, "(iiii)"),
    REG_NATIVE_FUNC(__kmpc_global_thread_num, "(i)i"),
    REG_NATIVE_FUNC(__kmpc_master, "(ii)i"),
    REG_NATIVE_FUNC(__kmpc_push_num_threads, "(iii)"),
    REG_NATIVE_FUNC(__kmpc_reduce, "(iiiiiii)i"),
    REG_NATIVE_FUNC(__kmpc_reduce_nowait, "(iiiiiii)i"),
    REG_NATIVE_FUNC(__kmpc_single, "(ii)i"),
    REG_NATIVE_FUNC(omp_get_max_threads, "()i"),
    REG_NATIVE_FUNC(omp_get_num_threads, "()i"),
    REG_NATIVE_FUNC(omp_get_thread_num, "()i"),
    REG_NATIVE_FUNC(omp_get_wtime, "()F"),
    REG_NATIVE_FUNC(omp_set_num_threads, "(i)"),
};

uint32_t getFaasmOpenMPApi(NativeSymbol** nativeSymbols)
{
    *nativeSymbols = ns;
    return sizeof(ns) / sizeof(NativeSymbol);
}
}
