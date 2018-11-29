#pragma once

#include <util/util.h>

#include <chrono>
#include <string>
#include <spdlog/spdlog.h>

#include <proto/faasm.pb.h>
#include <hiredis/hiredis.h>

namespace infra {
    const std::string COUNTER_PREFIX = "n_";
    const std::string PREWARM_QUEUE = "prewarm";
    const std::string PREWARM_COUNTER = "n_prewarm";
    const std::string COLD_QUEUE = "cold";
    const std::string COLD_COUNTER = "n_cold";

    /** Function utilities */
    std::string getFunctionFile(const message::Message &msg);

    std::string getFunctionObjectFile(const message::Message &msg);

    std::vector<uint8_t> getFunctionObjectBytes(const message::Message &msg);

    std::string getFunctionQueueName(const message::Message &msg);

    std::string getFunctionCounterName(const message::Message &msg);

    bool isValidFunction(const message::Message &msg);

    std::string funcToString(const message::Message &msg);

    message::Message buildPrewarmMessage(const message::Message &original);

    message::Message buildBindMessage(const message::Message &original);

    /** Serialisation */
    std::vector<uint8_t> messageToBytes(const message::Message &msg);

    class Redis {

    public:

        Redis();

        ~Redis();

        /** Returns the instance for the current thread */
        static Redis *getThreadConnection();

        std::vector<uint8_t> get(const std::string &key);

        void set(const std::string &key, const std::vector<uint8_t> &value);

        void del(const std::string &key);

        long getCounter(const std::string &key);

        long incr(const std::string &key);

        long decr(const std::string &key);

        void setRange(const std::string &key, long offset, const std::vector<uint8_t> &value);

        std::vector<uint8_t> getRange(const std::string &key, long start, long end);

        void enqueue(const std::string &queueName, const std::vector<uint8_t> &value);

        void enqueueMessage(const std::string &queueName, const message::Message &msg);

        std::vector<uint8_t> dequeue(const std::string &queueName, int timeout=util::DEFAULT_TIMEOUT);

        void flushAll();

        long listLength(const std::string &queueName);

        void callFunction(message::Message &msg);

        void addMoreWorkers(message::Message &msg, const std::string &queueName);

        message::Message nextMessage(const std::string &queueName, int timeout=util::DEFAULT_TIMEOUT);

        void setFunctionResult(message::Message &msg, bool success);

        message::Message getFunctionResult(const message::Message &msg);

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
