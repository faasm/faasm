#pragma once

#include "CoreRedis.h"

#include <util/config.h>

#include <hiredis/hiredis.h>
#include <unordered_set>
#include <spdlog/spdlog.h>
#include <string>
#include <thread>


namespace redis {
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


    class Redis : public CoreRedis {

    public:
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

        std::unordered_set<std::string> smembers(const std::string &key);

        std::unordered_set<std::string> sdiff(const std::string &keyA, const std::string &keyB);

        std::unordered_set<std::string> sinter(const std::string &keyA, const std::string &keyB);

        void flushAll();

        long listLength(const std::string &queueName);

        long getTtl(const std::string &key);

        void expire(const std::string &key, long expiry);

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
         * ------ Queueing ------
         */
        void enqueue(const std::string &queueName, const std::string &value);

        void enqueueBytes(const std::string &queueName, const std::vector<uint8_t> &value);

        std::string dequeue(const std::string &queueName, int timeout = util::DEFAULT_TIMEOUT);

        std::vector<uint8_t> dequeueBytes(const std::string &queueName, int timeout = util::DEFAULT_TIMEOUT);

    private:
        explicit Redis(const RedisInstance &instance);

        const RedisInstance &instance;

        redisReply* dequeueBase(const std::string &queueName, int timeout);
    };

    class RedisNoResponseException : public std::exception {
    };
};
