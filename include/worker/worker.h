#pragma once

#include <infra/infra.h>
#include <string>
#include <proto/faasm.pb.h>

namespace worker {

    class Worker {
    public:
        Worker();
        void start();

        void invokeFunction(message::FunctionCall &call);

    private:
        infra::RedisClient redis;
    };
}