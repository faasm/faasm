#include "Redis.h"

#include <util/network.h>
#include <util/environment.h>
#include <util/logging.h>
#include <util/random.h>

#include <thread>
#include <util/gids.h>

namespace redis {

    RedisInstance::RedisInstance(RedisRole roleIn) : role(roleIn) {
        util::SystemConfig &conf = util::getSystemConfig();

        if (role == STATE) {
            hostname = conf.redisStateHost;
            ip = util::getIPFromHostname(hostname);

        } else {
            hostname = conf.redisQueueHost;
            ip = util::getIPFromHostname(hostname);
        }

        std::string portStr = conf.redisPort;
        port = std::stoi(portStr);

        // Load scripts
        if (conditionalLockSha.empty()) {
            std::unique_lock<std::mutex> lock(scriptsLock);

            if (conditionalLockSha.empty()) {
                printf("Loading scripts for Redis instance at %s\n", hostname.c_str());
                redisContext *context = redisConnect(ip.c_str(), port);

                conditionalLockSha = this->loadScript(context, conditionalLockCmd);
                delifeqSha = this->loadScript(context, delifeqCmd);

                redisFree(context);
            }
        }
    }

    std::string RedisInstance::loadScript(redisContext *context, const std::string &scriptBody) {
        auto reply = (redisReply *) redisCommand(context, "SCRIPT LOAD %s", scriptBody.c_str());

        if(reply == nullptr) {
            throw std::runtime_error("Error loading script from Redis");
        }

        if (reply->type == REDIS_REPLY_ERROR) {
            throw std::runtime_error(reply->str);
        }

        std::string scriptSha = reply->str;
        freeReplyObject(reply);

        return scriptSha;
    }

    Redis::Redis(const RedisInstance &instanceIn) : instance(instanceIn) {
        // Note, connect with IP, not with hostname
        context = redisConnect(instance.ip.c_str(), instance.port);

        if (context == nullptr || context->err) {
            if (context) {
                printf("Error connecting to redis at %s: %s\n", instance.ip.c_str(), context->errstr);
            } else {
                printf("Error allocating redis context\n");
            }

            throw std::runtime_error("Failed to connect to redis");
        }

        printf("Connected to redis host %s at %s:%i\n", instance.hostname.c_str(), instance.ip.c_str(), instance.port);
    }

    Redis::~Redis() {
        redisFree(context);
    }

    /**
     *  ------ Utils ------
     */

    Redis &Redis::getState() {
        // Hiredis requires one instance per thread
        static RedisInstance stateInstance(STATE);
        static thread_local redis::Redis redisState(stateInstance);
        return redisState;
    }

    Redis &Redis::getQueue() {
        // Hiredis requires one instance per thread
        static RedisInstance queueInstance(QUEUE);
        static thread_local redis::Redis redisQueue(queueInstance);
        return redisQueue;
    }

    long getLongFromReply(redisReply *reply) {
        long res = 0;

        if (reply->str != nullptr) {
            res = std::stol(reply->str);
        }

        return res;
    }

    std::vector<uint8_t> getBytesFromReply(redisReply *reply) {
        // We have to be careful here to handle the bytes properly
        char *resultArray = reply->str;
        int resultLen = reply->len;

        std::vector<uint8_t> resultData(resultArray, resultArray + resultLen);

        return resultData;
    }

    void getBytesFromReply(redisReply *reply, uint8_t *buffer, size_t bufferLen) {
        // We have to be careful here to handle the bytes properly
        char *resultArray = reply->str;
        int resultLen = reply->len;

        if (resultLen > (int) bufferLen) {
            const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
            logger->error("Value ({}) too big for buffer ({})", resultLen, bufferLen);
            throw std::runtime_error("Reading value too big for buffer");
        }

        std::copy(resultArray, resultArray + resultLen, buffer);
    }

    /**
     *  ------ Lua scripts ------
     */

    long extractScriptResult(redisReply *reply) {
        if (reply->type == REDIS_REPLY_ERROR) {
            throw (std::runtime_error(reply->str));
        }

        long result = reply->integer;
        freeReplyObject(reply);

        return result;
    }

    /**
     *  ------ Standard Redis commands ------
     */

    void Redis::ping() {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        logger->debug("Pinging redis at {}", instance.hostname);
        auto reply = (redisReply *) redisCommand(context, "PING");

        std::string response(reply->str);

        freeReplyObject(reply);

        if (response != "PONG") {
            logger->debug("Failed pinging redis at {}", instance.hostname);
            throw std::runtime_error("Failed to ping redis host");
        }

        logger->debug("Successfully pinged redis");
    }

    void Redis::get(const std::string &key, uint8_t *buffer, size_t size) {
        auto reply = (redisReply *) redisCommand(context, "GET %s", key.c_str());
        getBytesFromReply(reply, buffer, size);
        freeReplyObject(reply);
    }

    std::vector<uint8_t> Redis::get(const std::string &key) {
        auto reply = (redisReply *) redisCommand(context, "GET %s", key.c_str());

        const std::vector<uint8_t> replyBytes = getBytesFromReply(reply);
        freeReplyObject(reply);

        return replyBytes;
    }

    long Redis::getCounter(const std::string &key) {
        auto reply = (redisReply *) redisCommand(context, "GET %s", key.c_str());

        if (reply == nullptr || reply->type == REDIS_REPLY_NIL || reply->len == 0) {
            return 0;
        }

        return std::stol(reply->str);
    }

    long Redis::incr(const std::string &key) {
        auto reply = (redisReply *) redisCommand(context, "INCR %s", key.c_str());

        long result = reply->integer;

        freeReplyObject(reply);
        return result;
    }

    long Redis::decr(const std::string &key) {
        auto reply = (redisReply *) redisCommand(context, "DECR %s", key.c_str());
        long result = reply->integer;
        freeReplyObject(reply);

        return result;
    }

    void Redis::set(const std::string &key, const std::vector<uint8_t> &value) {
        this->set(key, value.data(), value.size());
    }

    void Redis::set(const std::string &key, const uint8_t *value, size_t size) {
        auto reply = (redisReply *) redisCommand(context, "SET %s %b", key.c_str(), value, size);

        if (reply->type == REDIS_REPLY_ERROR) {
            const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
            logger->error("Failed to SET {} - {}", key.c_str(), reply->str);
        }

        freeReplyObject(reply);
    }

    void Redis::del(const std::string &key) {
        auto reply = (redisReply *) redisCommand(context, "DEL %s", key.c_str());
        freeReplyObject(reply);
    }

    void Redis::setRange(const std::string &key, long offset, const uint8_t *value, size_t size) {
        auto reply = (redisReply *) redisCommand(context, "SETRANGE %s %li %b", key.c_str(), offset, value,
                                                 size);
        freeReplyObject(reply);
    }

    void Redis::sadd(const std::string &key, const std::string &value) {
        auto reply = (redisReply *) redisCommand(context, "SADD %s %s", key.c_str(), value.c_str());
        freeReplyObject(reply);
    }

    void Redis::srem(const std::string &key, const std::string &value) {
        auto reply = (redisReply *) redisCommand(context, "SREM %s %s", key.c_str(), value.c_str());
        freeReplyObject(reply);
    }

    long Redis::scard(const std::string &key) {
        auto reply = (redisReply *) redisCommand(context, "SCARD %s", key.c_str());

        long res = reply->integer;

        freeReplyObject(reply);

        return res;
    }

    bool Redis::sismember(const std::string &key, const std::string &value) {
        auto reply = (redisReply *) redisCommand(context, "SISMEMBER %s %s", key.c_str(), value.c_str());

        bool res = reply->integer == 1;

        freeReplyObject(reply);

        return res;
    }

    std::string Redis::srandmember(const std::string &key) {
        auto reply = (redisReply *) redisCommand(context, "SRANDMEMBER %s", key.c_str());

        std::string res;
        if (reply->len > 0) {
            res = reply->str;
        }

        freeReplyObject(reply);

        return res;
    }

    std::unordered_set<std::string> extractStringSetFromReply(redisReply *reply) {
        std::unordered_set<std::string> retValue;
        for (size_t i = 0; i < reply->elements; i++) {
            retValue.insert(reply->element[i]->str);
        }

        freeReplyObject(reply);

        return retValue;
    }

    std::unordered_set<std::string> Redis::smembers(const std::string &key) {
        auto reply = (redisReply *) redisCommand(context, "SMEMBERS %s", key.c_str());
        return extractStringSetFromReply(reply);
    }

    std::unordered_set<std::string> Redis::sinter(const std::string &keyA, const std::string &keyB) {
        auto reply = (redisReply *) redisCommand(context, "SINTER %s %s", keyA.c_str(), keyB.c_str());
        return extractStringSetFromReply(reply);
    }

    std::unordered_set<std::string> Redis::sdiff(const std::string &keyA, const std::string &keyB) {
        auto reply = (redisReply *) redisCommand(context, "SDIFF %s %s", keyA.c_str(), keyB.c_str());
        return extractStringSetFromReply(reply);
    }

    void Redis::flushAll() {
        auto reply = (redisReply *) redisCommand(context, "FLUSHALL");
        freeReplyObject(reply);
    }

    long Redis::listLength(const std::string &queueName) {
        auto reply = (redisReply *) redisCommand(context, "LLEN %s", queueName.c_str());

        if (reply == nullptr || reply->type == REDIS_REPLY_NIL) {
            return 0;
        }

        long result = reply->integer;
        freeReplyObject(reply);

        return result;
    }

    long Redis::getTtl(const std::string &key) {
        auto reply = (redisReply *) redisCommand(context, "TTL %s", key.c_str());

        long ttl = reply->integer;
        freeReplyObject(reply);

        return ttl;
    }

    void Redis::expire(const std::string &key, long expiry) {
        auto reply = (redisReply *) redisCommand(context, "EXPIRE %s %d", key.c_str(), expiry);
        freeReplyObject(reply);
    }

    void Redis::refresh() {
        redisReconnect(context);
    }

    /**
     * Note that start/end are both inclusive
     */
    void Redis::getRange(const std::string &key, uint8_t *buffer, size_t bufferLen, long start, long end) {
        auto reply = (redisReply *) redisCommand(context, "GETRANGE %s %li %li", key.c_str(), start, end);

        // Importantly getrange is inclusive so we need to be checking the buffer length
        getBytesFromReply(reply, buffer, bufferLen);
        freeReplyObject(reply);
    }

    /**
     *  ------ Locking ------
     */

    long Redis::acquireConditionalLock(const std::string &key, long expectedValue) {
        std::string lockKey = key + "_lock";
        unsigned int lockId = util::generateGid();

        // Invoke the script
        auto reply = (redisReply *) redisCommand(
                context,
                "EVALSHA %s 2 %s %s %i %i",
                instance.conditionalLockSha.c_str(),
                key.c_str(),
                lockKey.c_str(),
                expectedValue,
                lockId
        );

        long result = extractScriptResult(reply);

        if (result > 0) {
            return lockId;
        } else {
            return result;
        }
    }

    long Redis::acquireLock(const std::string &key, int expirySeconds) {
        // Implementation of single host redlock algorithm
        // https://redis.io/topics/distlock
        unsigned int lockId = util::generateGid();

        std::string lockKey = key + "_lock";

        long result = this->setnxex(lockKey, lockId, expirySeconds);

        // Return the lock ID if successful, else return -1
        if (result == 1) {
            return lockId;
        } else {
            return 0;
        }
    }

    void Redis::releaseLock(const std::string &key, long lockId) {
        std::string lockKey = key + "_lock";
        this->delIfEq(lockKey, lockId);
    }

    void Redis::delIfEq(const std::string &key, long value) {
        // Invoke the script
        auto reply = (redisReply *) redisCommand(
                context,
                "EVALSHA %s 1 %s %i",
                instance.delifeqSha.c_str(),
                key.c_str(),
                value
        );

        extractScriptResult(reply);
    }

    bool Redis::setnxex(const std::string &key, long value, int expirySeconds) {
        // See docs on set for info on options: https://redis.io/commands/set
        // We use NX to say "set if not exists" and ex to specify the expiry of this key/value
        // This is useful in implementing locks. We only use longs as values to keep things simple
        auto reply = (redisReply *) redisCommand(context, "SET %s %i NX EX %i", key.c_str(), value, expirySeconds);

        if (reply->type == REDIS_REPLY_ERROR) {
            const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
            logger->error("Failed to SET {} - {}", key.c_str(), reply->str);
        }

        bool success = true;
        if (reply->type == REDIS_REPLY_NIL) {
            success = false;
        }

        freeReplyObject(reply);

        return success;
    }

    long Redis::getLong(const std::string &key) {
        auto reply = (redisReply *) redisCommand(context, "GET %s", key.c_str());

        long res = getLongFromReply(reply);
        freeReplyObject(reply);

        return res;
    }

    void Redis::setLong(const std::string &key, long value) {
        auto reply = (redisReply *) redisCommand(context, "SET %s %i", key.c_str(), value);

        freeReplyObject(reply);
    }

    /**
     *  ------ Queueing ------
     */

    void Redis::enqueue(const std::string &queueName, const std::string &value) {
        auto reply = (redisReply *) redisCommand(context, "RPUSH %s %s", queueName.c_str(), value.c_str());
        freeReplyObject(reply);
    }

    void Redis::enqueueBytes(const std::string &queueName, const std::vector<uint8_t> &value) {
        // NOTE: Here we must be careful with the input and specify bytes rather than a string
        // otherwise an encoded false boolean can be treated as a string terminator
        auto reply = (redisReply *) redisCommand(context, "RPUSH %s %b", queueName.c_str(), value.data(), value.size());

        if(reply->type != REDIS_REPLY_INTEGER) {
            throw std::runtime_error("Failed to enqueue bytes. Reply type = " + std::to_string(reply->type));
        } else if(reply->integer <= 0) {
            throw std::runtime_error("Failed to enqueue bytes. Length = " + std::to_string(reply->integer));
        }

        freeReplyObject(reply);
    }

    redisReply *Redis::dequeueBase(const std::string &queueName, int timeoutMs) {
        // Note, timeouts need to be converted into seconds
        int timeoutSecs = timeoutMs / 1000;
        auto reply = (redisReply *) redisCommand(context, "BLPOP %s %d", queueName.c_str(), timeoutSecs);

        if (reply == nullptr || reply->type == REDIS_REPLY_NIL) {
            throw RedisNoResponseException();
        } else if (reply->type != REDIS_REPLY_ARRAY) {
            throw std::runtime_error("Expected array response from BLPOP but got " + std::to_string(reply->type));
        }

        size_t nResults = reply->elements;

        if (nResults > 2) {
            throw std::runtime_error("Returned more than one pair of dequeued values");
        }

        return reply;
    }

    std::string Redis::dequeue(const std::string &queueName, int timeoutMs) {
        redisReply *reply = this->dequeueBase(queueName, timeoutMs);

        redisReply *r = reply->element[1];
        std::string result(r->str);

        freeReplyObject(reply);

        return result;
    }

    std::vector<uint8_t> Redis::dequeueBytes(const std::string &queueName, int timeoutMs) {
        redisReply *reply = this->dequeueBase(queueName, timeoutMs);

        // Note, BLPOP will return the queue name and the value returned (elements 0 and 1)
        redisReply *r = reply->element[1];

        const std::vector<uint8_t> replyBytes = getBytesFromReply(r);
        freeReplyObject(reply);

        return replyBytes;
    }
}

