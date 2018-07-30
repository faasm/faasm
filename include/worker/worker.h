#pragma once

#include <infra/infra.h>
#include <string>
#include <proto/faasm.pb.h>
#include <Runtime/Linker.h>


namespace worker {
    class WasmModule {
    public:
        WasmModule();

        int execute(message::FunctionCall &call);
    };

    class Worker {
    public:
        Worker();

        void start();

    private:
        infra::RedisClient redis;
    };
}