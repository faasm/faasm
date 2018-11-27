#pragma once

#include <chrono>
#include <string>
#include <spdlog/spdlog.h>

#include <proto/faasm.pb.h>
#include <hiredis/hiredis.h>

namespace infra {
    /** Function utilities */
    std::string getFunctionFile(const message::Message &call);

    std::string getFunctionObjectFile(const message::Message &call);

    std::vector<uint8_t> getFunctionObjectBytes(const message::Message &call);

    std::string getFunctionQueueName(const message::Message &call);

    std::string getFunctionSetName(const message::Message &call);

    bool isValidFunction(const message::Message &call);

    std::string funcToString(const message::Message &call);

    /** Serialisation */
    std::vector<uint8_t> callToBytes(const message::Message &call);

    /** Redis client */
    const std::string SET_PREFIX = "s_";
    const std::string PREWARM_QUEUE = "prewarm";
    const std::string PREWARM_SET = "s_prewarm";
    const std::string COLD_QUEUE = "cold";
    const std::string COLD_SET = "s_cold";

    static const int DEFAULT_TIMEOUT_SECONDS = 60;

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

        bool sismember(const std::string &key, const std::string &value);

        long scard(const std::string &key);

        std::string spop(const std::string &key);

        void setRange(const std::string &key, long offset, const std::vector<uint8_t> &value);

        std::vector<uint8_t> getRange(const std::string &key, long start, long end);

        void enqueue(const std::string &queueName, const std::vector<uint8_t> &value);

        std::vector<uint8_t> dequeue(const std::string &queueName, int timeout=DEFAULT_TIMEOUT_SECONDS);

        void flushAll();

        long listLength(const std::string &queueName);

        void callFunction(message::Message &call);

        message::Message nextMessage(const std::string &queueName, int timeout=DEFAULT_TIMEOUT_SECONDS);

        void setFunctionResult(message::Message &call, bool success);

        message::Message getFunctionResult(const message::Message &call);

        std::string addToFunctionSet(const message::Message &call, const std::string &queueName);

        void removeFromFunctionSet(const message::Message &call, const std::string &queueName);

        void addToUnassignedSet(const std::string &queueName);

        void removeFromUnassignedSet(const std::string &queueName);

        std::string getQueueForFunc(const message::Message &call);

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
