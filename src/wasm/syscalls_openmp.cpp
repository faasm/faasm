#include "WasmModule.h"
#include "syscalls.h"
#include "openmp_types.h"

#include <WAVM/Runtime/Runtime.h>
#include <WAVM/Runtime/Intrinsics.h>

namespace wasm {
    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "omp_get_thread_num", I32, omp_get_thread_num) {
        util::getLogger()->debug("S - omp_get_thread_num");

        return 1;
    }

    /**
     * Function defined in openmp/runtime/src/kmp_csupport.cpp
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

        // Here we want to execute functions on the module as we would normally but in a new context
        auto threadContext = createContext(getCompartmentFromContextRuntimeData(contextRuntimeData));
        IR::FunctionType funcType = Runtime::getFunctionType(func);
        IR::UntaggedValue result;
        std::vector<IR::UntaggedValue> arguments;
        Runtime::invokeFunction(threadContext, func, funcType, arguments.data(), &result);
    }

    void ompLink() {

    }
}
