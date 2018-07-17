#pragma once

#include <iostream>
#include <cpp_redis/cpp_redis>
#include <string>
#include <proto/faasm.pb.h>

namespace redis {
    class RedisClient : public cpp_redis::client {
    public:
        RedisClient();

        /** Check writing/ reading */
        std::string check(const std::string &value);

        /** Enqueue on FIFO */
        void enqueue(const std::string &queueName, const std::string &value);

        /** Enqueue on FIFO */
        void dequeue(const std::string &queueName, std::function<void(const std::string &)>);

        /** Adds a function call */
        void callFunction(message::FunctionCall &call);

        /** Gets next function call */
        void nextFunctionCall(std::function<void(message::FunctionCall &)>);
    };
}
