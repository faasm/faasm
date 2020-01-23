#include "WasmModule.h"
#include "syscalls.h"
#include "openmp_types.h"

#include <WAVM/Runtime/Runtime.h>
#include <WAVM/Runtime/Intrinsics.h>
#include <util/environment.h>

namespace wasm {
    static thread_local int thisThreadNumber = 0;
    static thread_local unsigned int thisSectionThreadCount = 0;

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "omp_get_thread_num", I32, omp_get_thread_num) {
        util::getLogger()->debug("S - omp_get_thread_num");
        return thisThreadNumber;
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "omp_get_num_threads", I32, omp_get_num_threads) {
        util::getLogger()->debug("S - omp_get_num_threads");
        return thisSectionThreadCount;
    }

    /**
     * "Real" Function defined in openmp/runtime/src/kmp_csupport.cpp
     * Actual implementation calls __kmp_fork_call under the hood (openmp/runtime/src/kmp_runtime.cpp)
     *
     * Note that microtaskPtr is a function pointer. The type of this function pointer is a microtask_t
     * which is defined in kmp_csupport.cpp.
     */
    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "__kmpc_fork_call", void, __kmpc_fork_call, I32 locPtr, I32 argc,
                                   I32 microtaskPtr, I32 argsPtr) {
        // Looks like we don't need the source loc info (accessing it commented out below though)
        // Runtime::Memory *memoryPtr = getExecutingModule()->defaultMemory;
        // wasm_ident *wasmLoc = &Runtime::memoryRef<wasm_ident>(memoryPtr, locPtr);
        // logger->debug("wasmLoc.psource = {}", wasmLoc->psource);

        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->debug("S - __kmpc_fork_call {} {} {} {}", locPtr, argc, microtaskPtr, argsPtr);

        unsigned int numThreads = util::getUsableCores();

        // Retrieve the function from the table
        Runtime::Object *funcObj = Runtime::getTableElement(getExecutingModule()->defaultTable, microtaskPtr);
        Runtime::Function *func = Runtime::asFunction(funcObj);
        IR::FunctionType funcType = Runtime::getFunctionType(func);

        // Spawn function calls in threads in a loop
        std::vector<std::thread> threads;
        for (int threadNum = 0; threadNum < numThreads; threadNum++) {
            WasmModule *parentModule = getExecutingModule();
            message::Message *parentCall = getExecutingCall();

            // TODO - check this is correct
            // If we have no arguments to pass to the nested function, it only expects 2.
            // If we have more, we need to say how many and pass a pointer to them.
            std::vector<IR::UntaggedValue> arguments;
            if(argc > 0) {
                arguments = {thisThreadNumber, argc, argsPtr};
            } else {
                arguments = {thisThreadNumber, 0};
            }

            threads.emplace_back([
                    threadNum, numThreads,
                    func, funcType, arguments,
                    contextRuntimeData, parentModule, parentCall
                    ] {
                // Note that executing module and call are stored in TLS so need to explicitly set here
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
        thisSectionThreadCount = 0;
    }

    void ompLink() {

    }
}
