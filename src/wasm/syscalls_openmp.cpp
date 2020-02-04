#include "WasmModule.h"

#include <WAVM/Platform/Thread.h>
#include <WAVM/Runtime/Runtime.h>
#include <WAVM/Runtime/Intrinsics.h>
#include <util/environment.h>

#include <condition_variable>
#include <mutex>

namespace wasm {
    // Thread-local variables for each OMP thread
    static thread_local int thisThreadNumber = 0;
    static thread_local unsigned int thisSectionThreadCount = 1;

    // Global variables controlled by master
    static int masterNumThread = -1;

    // Locking/ barriers
    static int numThreadSemaphore = -1;
    static std::mutex barrierMutex;
    static std::condition_variable barrierCV;

    // Threads currently in action
    std::vector<WAVM::Platform::Thread *> platformThreads;

    // Arguments passed to spawned threads
    struct OMPThreadArgs {
        int tid;
        Runtime::ContextRuntimeData *contextRuntimeData;
        wasm::WasmModule *parentModule;
        message::Message *parentCall;
        unsigned int sectionThreadCount;
        Runtime::Function *func;
        IR::UntaggedValue *funcArgs;
    };

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
        thisSectionThreadCount = threadArgs->sectionThreadCount;

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
        return thisSectionThreadCount;
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
    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "__kmpc_barrier", void, __kmpc_barrier, I32 loc, I32 global_tid) {
        util::getLogger()->debug("S - __kmpc_barrier {} {}", loc, global_tid);

        if (thisSectionThreadCount <= 1) {
            return;
        }

        {
            std::unique_lock<std::mutex> lk(barrierMutex);
            numThreadSemaphore--;
            barrierCV.wait(lk, [] { return numThreadSemaphore == 0; });
        }

        barrierCV.notify_all();
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
    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "__kmpc_master", I32, __kmpc_master, I32 loc, I32 global_tid) {
        util::getLogger()->debug("S - __kmpc_master {} {}", loc, global_tid);
        return thisThreadNumber == 0 ? 1 : 0;
    }

    /**
     * Only called by the thread executing the master region.
     * @param loc  source location information.
     * @param global_tid  global thread number .
     */
    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "__kmpc_end_master", void, __kmpc_end_master, I32 loc, I32 global_tid) {
        util::getLogger()->debug("S - __kmpc_end_master {} {}", loc, global_tid);
        WAVM_ASSERT(global_tid == 0 && thisThreadNumber == 0)
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "__kmpc_push_num_threads", void, __kmpc_push_num_threads,
                                   I32 loc, I32 global_tid, I32 num_threads) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->debug("S - __kmpc_push_num_threads {} {} {}", loc, global_tid, num_threads);

        if (num_threads > 0) {
            masterNumThread = num_threads;
        }
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "__kmpc_global_thread_num", I32, __kmpc_global_thread_num,
                                   I32 loc) {
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
        int numThreads = masterNumThread > 0 ? masterNumThread : util::getUsableCores();
        numThreadSemaphore = numThreads;

        // Set up TLS for master thread
        thisSectionThreadCount = numThreads;

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
                                         .sectionThreadCount = thisSectionThreadCount, .func = func,
                                         .funcArgs = microtaskArgs[threadNum].data()
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

        // Reset the master thread variables
        thisSectionThreadCount = 1;
        numThreadSemaphore = 1;
    }

    void ompLink() {

    }
}
