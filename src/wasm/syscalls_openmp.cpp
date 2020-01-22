#include "WasmModule.h"
#include "syscalls.h"
#include "openmp_types.h"

#include <WAVM/Runtime/Runtime.h>
#include <WAVM/Runtime/Intrinsics.h>
#include <util/environment.h>

namespace wasm {
    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "omp_get_thread_num", I32, omp_get_thread_num) {
        util::getLogger()->debug("S - omp_get_thread_num");

        // Just return current tid for debugging purposes now
        auto tid = (pid_t) syscall(SYS_gettid);

        return (I32) tid;
    }

    /**
     * Function defined in openmp/runtime/src/kmp_csupport.cpp
     *
     * Actual implementation calls __kmp_fork_call under the hood (openmp/runtime/src/kmp_runtime.cpp)
     */
    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "__kmpc_fork_call", void, __kmpc_fork_call, I32 locPtr, I32 argc,
                                   I32 microtaskPtr, I32 argsPtr) {
        // Extract the source location from the wasm memory
        Runtime::Memory *memoryPtr = getExecutingModule()->defaultMemory;
        wasm_ident *wasmLoc = &Runtime::memoryRef<wasm_ident>(memoryPtr, locPtr);

        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->debug("S - __kmpc_fork_call {} {} {} {}", locPtr, argc, microtaskPtr, argsPtr);
        logger->debug("S - __kmpc_fork_call wasmLoc.psource = {}", wasmLoc->psource);

        // Note that microtaskPtr here is a function pointer and index in the wasm function table
        logger->debug("S - __kmpc_fork_call microtaskPtr = {}", microtaskPtr);

        // Retrieve the function from the table
        Runtime::Object *funcObj = Runtime::getTableElement(getExecutingModule()->defaultTable, microtaskPtr);
        Runtime::Function *func = Runtime::asFunction(funcObj);
        IR::FunctionType funcType = Runtime::getFunctionType(func);

        unsigned int nCores = util::getUsableCores();

        // Spawn function calls in threads in a loop
        std::vector<std::thread> threads;
        for (int i = 0; i < nCores; i++) {
            WasmModule *parentModule = getExecutingModule();
            message::Message *parentCall = getExecutingCall();
            threads.emplace_back([func, funcType, contextRuntimeData, parentModule, parentCall] {
                // Note that executing module and call are stored in TLS so need to explicitly set here
                setExecutingModule(parentModule);
                setExecutingCall(parentCall);

                // Create a new context for this thread
                auto threadContext = createContext(getCompartmentFromContextRuntimeData(contextRuntimeData));

                // Execute the function as we would normally
                IR::UntaggedValue result;
                std::vector<IR::UntaggedValue> arguments = {0, 0};
                Runtime::invokeFunction(threadContext, func, funcType, arguments.data(), &result);
            });
        }

        // Await all threads
        for (auto &t: threads) {
            if (t.joinable()) {
                t.join();
            }
        }
    }

    void ompLink() {

    }
}
