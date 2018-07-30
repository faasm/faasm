#pragma once

#include <iostream>
#include <cpp_redis/cpp_redis>
#include <string>
#include <proto/faasm.pb.h>


namespace infra {
    /** Function utilities */
    std::string getFunctionFile(message::FunctionCall &call);

    /** Low-level Redis interaction */
    class RedisClient : public cpp_redis::client {

    public:
        RedisClient();

        void connect();

        /** Check writing/ reading */
        std::string check(const std::string &value);

        /** Enqueue on FIFO */
        void enqueue(const std::string &queueName, const std::string &value);

        /** Enqueue on FIFO */
        std::vector<std::string> blockingDequeue(const std::string &queueName);

        /** Adds a function call */
        void callFunction(const message::FunctionCall &call);

        /** Gets next function call */
        message::FunctionCall nextFunctionCall();

        void setFunctionResult(message::FunctionCall &call, bool success);

        message::FunctionCall getFunctionResult(const message::FunctionCall &call);
    };
}
