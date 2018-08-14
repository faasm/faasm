#pragma once

#include <iostream>
#include <string>
#include <proto/faasm.pb.h>
#include <hiredis/hiredis.h>


namespace infra {
    /** Function utilities */
    std::string getFunctionFile(message::FunctionCall &call);

    /** Redis client */
    class Redis {

    public:

        Redis();

        void connect();

        void enqueue(const std::string &queueName, const std::string &value);

        std::string dequeue(const std::string &queueName);

        void flushAll();

        long listLength(const std::string &queueName);

        void callFunction(message::FunctionCall &call);

        message::FunctionCall nextFunctionCall();

        void setFunctionResult(message::FunctionCall &call, bool success);

        message::FunctionCall getFunctionResult(const message::FunctionCall &call);

    private:
        redisContext *context;
    };
}
