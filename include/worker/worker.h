#pragma once

#include <infra/infra.h>

#include <string>
#include <exception>

#include <proto/faasm.pb.h>
#include <Runtime/Runtime.h>

using namespace IR;
using namespace Runtime;

namespace worker {
    const std::string ENTRYPOINT_FUNC = "run";
    const int INPUT_MAX_BYTES = 1024 * 1024;
    const int OUTPUT_MAX_BYTES = 1024 * 1024;

    /** Wrapper for wasm code */
    class WasmModule {
    public:
        WasmModule();

        /** Executes the function and stores the result */
        int execute(message::FunctionCall &call);

        /** Cleans up */
        void clean();

    private:
        ModuleInstance *moduleInstance;
        ValueTuple functionResults;

        I32 inputStart;
        I32 inputLength;
        I32 outputStart;
        I32 outputLength;
    };

    /** Worker wrapper */
    class Worker {
    public:
        Worker();

        void start();

        /** Called when one function wants to make call into another */
        static void chainFunction(U8 *userName,  U8 *funcName, U8* inputData, I32 inputLength) ;
    private:
        static infra::RedisClient redis;
    };

    /** Exceptions */
    class WasmException : public std::exception {
    };

    class InvalidResultException : public std::exception {
    };
}