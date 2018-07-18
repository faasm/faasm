#pragma once

#include <redis/redis.h>
#include <string>
#include <proto/faasm.pb.h>

namespace worker {

    class Worker {
    public:
        Worker();
        void start();

    private:
        redis::RedisClient redis;

        void executeFunction(message::FunctionCall &func);
    };
}