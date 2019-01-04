#pragma once

#include <util/util.h>

#include <string>
#include <spdlog/spdlog.h>

#include <thread>

#include <proto/faasm.pb.h>
#include <hiredis/hiredis.h>

namespace infra {

    std::string getFunctionFile(const message::Message &msg);

    std::string getFunctionObjectFile(const message::Message &msg);

    std::vector<uint8_t> getFunctionObjectBytes(const message::Message &msg);

    bool isValidFunction(const message::Message &msg);

    std::string funcToString(const message::Message &msg);

    std::vector<uint8_t> messageToBytes(const message::Message &msg);

    enum RedisRole {
        QUEUE,
        STATE,
    };

    class Redis {

    public:

        Redis(const RedisRole &role);

        ~Redis();

        /**
        *  ------ Utils ------
        */

        static Redis *getThreadQueue();

        static Redis *getThreadState();

        /**
        *  ------ Standard Redis commands ------
        */

        std::vector<uint8_t> get(const std::string &key);

        void get(const std::string &key, uint8_t *buffer, size_t size);

        void set(const std::string &key, const std::vector<uint8_t> &value);

        void set(const std::string &key, const uint8_t *value, size_t size);

        void del(const std::string &key);

        long getCounter(const std::string &key);

        long incr(const std::string &key);

        long decr(const std::string &key);

        void setRange(const std::string &key, long offset, const uint8_t *value, size_t size);

        void getRange(const std::string &key, uint8_t *buffer, size_t bufferLen, long start, long end);

        void sadd(const std::string &key, const std::string &value);

        void srem(const std::string &key, const std::string &value);

        long scard(const std::string &key);

        bool sismember(const std::string &key, const std::string &value);

        std::string srandmember(const std::string &key);

        std::vector<std::string> sdiff(const std::string &keyA, const std::string &keyB);

        std::vector<std::string> sinter(const std::string &keyA, const std::string &keyB);

        void flushAll();

        long listLength(const std::string &queueName);

        long getTtl(const std::string &key);

        /**
        *  ------ Locking ------
        */

        long acquireConditionalLock(const std::string &key, long expectedValue);

        long acquireLock(const std::string &key, int expirySeconds);

        void releaseLock(const std::string &key, long lockId);

        void delIfEq(const std::string &key, long value);

        bool setnxex(const std::string &key, long value, int expirySeconds);

        long getLong(const std::string &key);

        void setLong(const std::string &key, long value);

        /**
        *  ------ Queueing ------
        */

        void enqueue(const std::string &queueName, const std::vector<uint8_t> &value);

        void enqueueMessage(const std::string &queueName, const message::Message &msg);

        std::vector<uint8_t> dequeue(const std::string &queueName, int timeout = util::DEFAULT_TIMEOUT);

        /**
        *  ------ Function messages ------
        */

        message::Message nextMessage(const std::string &queueName, int timeout = util::DEFAULT_TIMEOUT);

        void setFunctionResult(message::Message &msg, bool success);

        message::Message getFunctionResult(const message::Message &msg);

    private:
        redisContext *context;
        std::string hostname;
        std::string port;

        std::string loadScript(const std::string &scriptBody);
    };

    class RedisNoResponseException : public std::exception {
    };

    // Scheduling
    const std::string GLOBAL_WORKER_SET = "workers";

    class Scheduler {
    public:
        Scheduler();

        static const int scheduleWaitMillis = 100;

        static const int scheduleRecursionLimit = 10;

        static long getFunctionCount(const message::Message &msg);

        static int getWorkerTimeout(const std::string &currentQueue);

        static void workerFinished(const std::string &currentQueue);

        static std::string callFunction(message::Message &msg);

        static std::string getFunctionQueueName(const message::Message &msg);

        static std::string getFunctionWorkerSetName(const message::Message &msg);

        static std::string getHostName();

        static std::string getHostPrewarmQueue(const std::string &hostName);

        static std::string getHostPrewarmQueue();

        static std::string getPrewarmQueueForFunction(const message::Message &msg, bool affinity);

    private:
        static void updateWorkerAllocs(const message::Message &msg, int recursionCount=0);

        static std::string getFunctionCounterName(const message::Message &msg);
    };
};
