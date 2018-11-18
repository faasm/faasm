#pragma once

#include <chrono>
#include <string>
#include <spdlog/spdlog.h>

#include <proto/faasm.pb.h>
#include <hiredis/hiredis.h>

namespace infra {
    /** Function utilities */
    std::string getFunctionFile(const message::FunctionCall &call);

    std::string getFunctionObjectFile(const message::FunctionCall &call);

    std::vector<uint8_t> getFunctionObjectBytes(const message::FunctionCall &call);

    bool isValidFunction(const message::FunctionCall &call);

    /** Serialisation */
    std::vector<uint8_t> callToBytes(const message::FunctionCall &call);

    /** Redis client */
    class Redis {

    public:

        Redis();

        ~Redis();

        /** Returns the instance for the current thread */
        static Redis *getThreadConnection();

        std::vector<uint8_t> get(const std::string &key);

        void set(const std::string &key, const std::vector<uint8_t> &value);

        void del(const std::string &key);

        void sadd(const std::string &key, const std::string &value);

        void srem(const std::string &key, const std::string &value);

        std::string spop(const std::string &key);

        void setRange(const std::string &key, long offset, const std::vector<uint8_t> &value);

        std::vector<uint8_t> getRange(const std::string &key, long start, long end);

        void enqueue(const std::string &queueName, const std::vector<uint8_t> &value);

        std::vector<uint8_t> dequeue(const std::string &queueName);

        void flushAll();

        long listLength(const std::string &queueName);

        void callFunction(message::FunctionCall &call);

        message::FunctionCall nextFunctionCall(const std::string &queueName);

        void setFunctionResult(message::FunctionCall &call, bool success);

        message::FunctionCall getFunctionResult(const message::FunctionCall &call);

        void addToFunctionSet(const message::FunctionCall &call, const std::string &queueName);

        void removeFromFunctionSet(const message::FunctionCall &call, const std::string &queueName);

        void addToUnassignedSet(const std::string &queueName);

        void removeFromUnassignedSet(const std::string &queueName);

        std::string getQueueForFunc(const message::FunctionCall &call);

        long getTtl(const std::string &key);

        void refresh();

    private:
        redisContext *context;
        std::string hostname;
        std::string port;
    };

    class RedisNoResponseException : public std::exception {
    };

    // Timing
    std::chrono::steady_clock::time_point startTimer();

    void logEndTimer(const std::string &label, const std::chrono::steady_clock::time_point &begin);
};
