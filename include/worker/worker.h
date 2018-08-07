#pragma once

#include <infra/infra.h>

#include <string>
#include <exception>

#include <proto/faasm.pb.h>
#include <Runtime/Linker.h>

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
        int execute(message::FunctionCall &call, std::vector<U8> &input);

        /** Cleans up */
        void clean();

        std::vector<I32> getOutputData();

    private:
        ModuleInstance *moduleInstance;
        ValueTuple functionResults;

        Context *context;

        std::vector<U8> inputData;
        std::vector<U8> outputData;

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

    private:
        infra::RedisClient redis;
    };

    /** Exceptions */
    class WasmException : public std::exception {
    };

    class InvalidResultException : public std::exception {
    };
}