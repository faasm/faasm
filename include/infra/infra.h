#pragma once

#include <util/util.h>

#include <string>
#include <spdlog/spdlog.h>

#include <thread>

#include <proto/faasm.pb.h>
#include <hiredis/hiredis.h>

#define GLOBAL_WORKER_SET "available_workers"

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

    class RedisInstance {
    public:
        explicit RedisInstance(RedisRole role);

        std::string conditionalLockSha;
        std::string delifeqSha;

        std::string ip;
        std::string hostname;
        int port;
    private:
        RedisRole role;

        std::mutex scriptsLock;

        std::string loadScript(redisContext *context, const std::string &scriptBody);

        // Conditional lock is used as part of a locking scheme where we need to make sure an assumption
        // is correct before acquiring the lock. The function takes 4 arguments:
        // key_to_check, key_to_lock, expected_value, lock_id
        //
        // This takes elements of the redlock algorithm described here: https://redis.io/topics/distlock
        //
        // Return values:
        // -1 = value is different
        // 0 = lock not acquired
        // 1 = lock acquired
        const std::string conditionalLockCmd = "local val = redis.call(\"GET\", KEYS[1]) \n"
                                               ""
                                               "if (val == false) or (val == ARGV[1]) then \n"
                                               "    return redis.call(\"SETNX\", KEYS[2], ARGV[2]) \n"
                                               "else \n"
                                               "    return -1 \n"
                                               "end";

        // Script to delete a key if it equals a given value
        const std::string delifeqCmd = "if redis.call(\"GET\", KEYS[1]) == ARGV[1] then \n"
                                       "    return redis.call(\"DEL\", KEYS[1]) \n"
                                       "else \n"
                                       "    return 0 \n"
                                       "end";
    };


    class Redis {

    public:
        ~Redis();

        /**
        *  ------ Factories ------
        */

        static Redis &getQueue();

        static Redis &getState();

        /**
        *  ------ Standard Redis commands ------
        */
        void ping();

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
        explicit Redis(const RedisInstance &instance);

        const RedisInstance &instance;

        redisContext *context;
    };

    class RedisNoResponseException : public std::exception {
    };

    // Scheduling
    class Scheduler {
    public:
        Scheduler();

        const int scheduleWaitMillis = 100;

        const int scheduleRecursionLimit = 10;

        long getFunctionCount(const message::Message &msg);

        void workerBound(const message::Message &msg);

        void workerUnbound(const message::Message &msg);

        void addCurrentHostToWorkerPool();

        std::string callFunction(message::Message &msg);

        std::string getFunctionQueueName(const message::Message &msg);

        std::string getFunctionWorkerSetName(const message::Message &msg);

        std::string getHostPrewarmQueue(const std::string &hostName);

        std::string getHostPrewarmQueue();

        std::string getPrewarmQueueForFunction(const message::Message &msg, bool affinity);

    private:
        std::map<std::string, int> funcCounts;
        std::mutex funcCountMutex;
        std::string hostname;

        void updateWorkerAllocs(const message::Message &msg, int recursionCount = 0);

        std::string getFunctionCounterName(const message::Message &msg);

        Redis &redis;
        util::SystemConfig &conf;
    };

    Scheduler& getScheduler();
};
