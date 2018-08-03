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

        int execute(message::FunctionCall &call);

        void printMemory(Uptr offset);
        void clean();

    private:
        Runtime::ModuleInstance *moduleInstance;
    };

    /** Worker wrapper */
    class Worker {
    public:
        Worker();

        void start();

    private:
        infra::RedisClient redis;
    };

    /** Exception for wasm failure */
    class WasmException: public std::exception {};
}