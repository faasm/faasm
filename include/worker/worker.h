#pragma once

#include <infra/infra.h>

#include <string>
#include <exception>

#include <proto/faasm.pb.h>
#include <Runtime/Linker.h>

namespace worker {
    /** Wrapper for wasm code */
    class WasmModule {
    public:
        WasmModule();

        /** Executes the function and stores the result */
        void execute(message::FunctionCall &call);

        /** Retrieves the result as a char pointer */
        char *resultToCharPtr();

        /** Cleans up */
        void clean();

    private:
        Runtime::ModuleInstance *moduleInstance;
        IR::ValueTuple functionResults;
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