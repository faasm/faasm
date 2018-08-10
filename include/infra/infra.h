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
        static std::vector<std::string> getKeyValueFromReply(cpp_redis::reply &reply);

        RedisClient();

        void connect();

        /** Check writing/ reading */
        std::string check(const std::string &value);

        /** Enqueue on FIFO */
        void enqueue(const std::string &queueName, const std::string &value);

        /** Blocking dequeue on FIFO */
        std::vector<std::string> blockingDequeue(const std::string &queueName);

        /** Non-blocking dequeue on FIFO */
        void dequeue(const std::string &queueName, const reply_callback_t &callback);

        /** Adds a function call */
        void callFunction(message::FunctionCall &call);

        /** Gets next function call */
        message::FunctionCall nextFunctionCall();

        void setFunctionResult(message::FunctionCall &call, bool success);

        void getFunctionResult(const message::FunctionCall &call,
                               const std::function<void(message::FunctionCall &)> &callback);

        message::FunctionCall blockingGetFunctionResult(const message::FunctionCall &call);
    };
}
