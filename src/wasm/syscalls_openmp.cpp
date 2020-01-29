#include "WasmModule.h"

#include <WAVM/Runtime/Runtime.h>
#include <WAVM/Runtime/Intrinsics.h>
#include <util/environment.h>

#include <condition_variable>
#include <mutex>
#include <stdio.h>

namespace wasm {
    static thread_local int thisThreadNumber = 0;
    static thread_local unsigned int thisSectionThreadCount = 1;
    static int masterNumThread = -1;
    static std::vector<std::thread> threads;
    static int numThreadSemaphore = -1;
    static std::mutex barrierMutex;
    static std::condition_variable barrierCV;

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
            barrierCV.wait(lk, []{return numThreadSemaphore == 0;});
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
        // The source loc info seems useless, but can be accessed with:
        // Runtime::Memory *memoryPtr = getExecutingModule()->defaultMemory;
        // wasm_ident *wasmLoc = &Runtime::memoryRef<wasm_ident>(memoryPtr, locPtr);
        // logger->debug("wasmLoc.psource = {}", wasmLoc->psource);

        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->debug("S - __kmpc_fork_call {} {} {} {}", locPtr, argc, microtaskPtr, argsPtr);

        // Retrieve the microstask function from the table
        Runtime::Object *funcObj = Runtime::getTableElement(getExecutingModule()->defaultTable, microtaskPtr);
        Runtime::Function *func = Runtime::asFunction(funcObj);
        IR::FunctionType funcType = Runtime::getFunctionType(func);

        // Get reference to module memory
        Runtime::GCPointer<Runtime::Memory> &memoryPtr = getExecutingModule()->defaultMemory;
        
        // Spawn calls to the microtask in multiple threads
        int numThreads = masterNumThread > 0 ? masterNumThread : util::getUsableCores();
        // Saves number of thread separately in case threads modify it before done spawning
        numThreadSemaphore = numThreads;
        for (int threadNum = 0; threadNum < numThreads; threadNum++) {
            WasmModule *parentModule = getExecutingModule();
            message::Message *parentCall = getExecutingCall();

            // Here we need to build up the arguments for this function (one 
            // argument per non-global shared variable). 
            std::vector<IR::UntaggedValue> arguments = {thisThreadNumber, argc};
            if(argc > 0) {
                // Get pointer to start of arguments in host memory
                U32 *pointers = Runtime::memoryArrayPtr<U32>(memoryPtr, argsPtr, argc);
                for(int argIdx = 0; argIdx < argc; argIdx++) {
                    arguments.push_back(pointers[argIdx]);
                }
            }

            threads.emplace_back([
                    threadNum, numThreads,
                    func, funcType, arguments,
                    contextRuntimeData, parentModule, parentCall
                    ] {
                // Note that the executing module and call are stored in TLS so need to explicitly set here
                setExecutingModule(parentModule);
                setExecutingCall(parentCall);

                // Set the thread count for this section
                thisSectionThreadCount = numThreads;

                // Assign this thread its relevant thread number
                thisThreadNumber = threadNum;

                // Create a new context for this thread
                auto threadContext = createContext(getCompartmentFromContextRuntimeData(contextRuntimeData));

                IR::UntaggedValue result;
                Runtime::invokeFunction(threadContext, func, funcType, arguments.data(), &result);

                // TODO - check the result here
            });
        }

        // Await all threads
        for (auto &t: threads) {
            if (t.joinable()) {
                t.join();
            }
        }

        // Reset the thread count for this thread
        thisSectionThreadCount = 1;
    }

    void ompLink() {

    }
}
