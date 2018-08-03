#pragma once

#include <infra/infra.h>

#include <string>
#include <exception>

#include <proto/faasm.pb.h>
#include <Runtime/Linker.h>
#include <Emscripten/Emscripten.h>

namespace worker {
    /** Wrapper for wasm code */
    class WasmModule {
    public:
        WasmModule();

        int execute(message::FunctionCall &call);

        void printMemory(int ptr);
        void clean();

    private:
        Runtime::ModuleInstance *moduleInstance;
        Emscripten::Instance *emscriptenInstance;
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