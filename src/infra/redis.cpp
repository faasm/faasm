#include "infra.h"
#include "util/util.h"
#include "prof/prof.h"


#include <thread>

namespace infra {
    // Note - hiredis redis contexts are suitable only for single threads
    // therefore we need to ensure that each thread has its own instance
    static thread_local infra::Redis redisState(STATE);
    static thread_local infra::Redis redisQueue(QUEUE);

    // Once we have resolved the IP of the redis instance, we need to keep using it
    // This allows things operating within the network namespace to resolve it properly
    static std::string redisStateIp;
    static std::string redisQueueIp;

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
    static std::string conditionalLockSha;
    static std::string conditionalLockCmd = "local val = redis.call(\"GET\", KEYS[1]) \n"
                                            ""
                                            "if (val == false) or (val == ARGV[1]) then \n"
                                            "    return redis.call(\"SETNX\", KEYS[2], ARGV[2]) \n"
                                            "else \n"
                                            "    return -1 \n"
                                            "end";



    // Script to delete a key if it equals a given value
    static std::string delifeqSha;
    static std::string delifeqCmd = "if redis.call(\"GET\", KEYS[1]) == ARGV[1] then \n"
                                    "    return redis.call(\"DEL\", KEYS[1]) \n"
                                    "else \n"
                                    "    return 0 \n"
                                    "end";

    Redis::Redis(const RedisRole &role) {
        std::string thisIp;
        if (role == STATE) {
            hostname = util::getEnvVar("REDIS_STATE_HOST", "localhost");

            if (redisStateIp.empty()) {
                redisStateIp = util::getIPFromHostname(hostname);
            }

            thisIp = redisStateIp;
        } else {
            hostname = util::getEnvVar("REDIS_QUEUE_HOST", "localhost");

            if (redisQueueIp.empty()) {
                redisQueueIp = util::getIPFromHostname(hostname);
            }

            thisIp = redisQueueIp;
        }

        port = util::getEnvVar("REDIS_PORT", "6379");

        // Note, connect with IP, not with hostname
        int portInt = std::stoi(port);

        context = redisConnect(thisIp.c_str(), portInt);

        if (context == nullptr || context->err) {
            if (context) {
                printf("Error connecting to redis at %s: %s\n", thisIp.c_str(), context->errstr);
            } else {
                printf("Error allocating redis context\n");
            }

            throw std::runtime_error("Failed to connect to redis");
        }
    }

    Redis::~Redis() {
        redisFree(context);
    }

    /**
     *  ------ Utils ------
     */

    Redis *Redis::getThreadState() {
        return &redisState;
    }

    Redis *Redis::getThreadQueue() {
        return &redisQueue;
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

        if (resultLen > bufferLen) {
            throw std::runtime_error("Reading value too big for buffer");
        }

        std::copy(resultArray, resultArray + resultLen, buffer);
    }

    /**
     *  ------ Lua script management ------
     */

    std::string Redis::loadScript(const std::string &scriptBody) {
        auto reply = (redisReply *) redisCommand(context, "SCRIPT LOAD %s", scriptBody.c_str());
        if (reply->type == REDIS_REPLY_ERROR) {
            throw (std::runtime_error(reply->str));
        }

        std::string scriptSha = reply->str;
        freeReplyObject(reply);

        return scriptSha;
    }

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

        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        if (reply->type == REDIS_REPLY_ERROR) {
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
        if(reply->len > 0) {
            res = reply->str;
        }

        freeReplyObject(reply);

        return res;
    }

    std::vector<std::string> extractStringListFromReply(redisReply *reply) {
        std::vector<std::string> retValue;
        for(int i = 0; i < reply->elements; i++) {
            retValue.emplace_back(reply->element[i]->str);
        }

        freeReplyObject(reply);

        return retValue;
    }

    std::vector<std::string> Redis::sinter(const std::string &keyA, const std::string &keyB) {
        auto reply = (redisReply *) redisCommand(context, "SINTER %s %s", keyA.c_str(), keyB.c_str());
        return extractStringListFromReply(reply);
    }

    std::vector<std::string> Redis::sdiff(const std::string &keyA, const std::string &keyB) {
        auto reply = (redisReply *) redisCommand(context, "SDIFF %s %s", keyA.c_str(), keyB.c_str());
        return extractStringListFromReply(reply);
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
    *  ------ Scheduling ------
    */

    std::pair<long, long> Redis::mgetLongPair(const std::string &keyA, const std::string &keyB) {
        auto reply = (redisReply *) redisCommand(context, "MGET %s %s", keyA.c_str(), keyB.c_str());

        long valA = getLongFromReply(reply->element[0]);
        long valB = getLongFromReply(reply->element[1]);

        freeReplyObject(reply);

        return std::pair<long, long>(valA, valB);
    }

    /**
     *  ------ Locking ------
     */

    long Redis::acquireConditionalLock(const std::string &key, long expectedValue) {
        if (conditionalLockSha.empty()) {
            const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
            conditionalLockSha = this->loadScript(conditionalLockCmd);
            logger->debug("Loaded conditional lock script with SHA {}", conditionalLockSha);
        };

        std::string lockKey = key + "_lock";
        int lockId = util::randomInteger(0, 100000);

        // Invoke the script
        auto reply = (redisReply *) redisCommand(
                context,
                "EVALSHA %s 2 %s %s %i %i",
                conditionalLockSha.c_str(),
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
        int lockId = util::randomInteger(0, 100000);

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
        // Create the script if not exists
        if (delifeqSha.empty()) {
            const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
            delifeqSha = this->loadScript(delifeqCmd);
            logger->debug("Loaded del if eq script with SHA {}", delifeqSha);
        };

        // Invoke the script
        auto reply = (redisReply *) redisCommand(context, "EVALSHA %s 1 %s %i", delifeqSha.c_str(),
                                                 key.c_str(), value);

        extractScriptResult(reply);
    }

    bool Redis::setnxex(const std::string &key, long value, int expirySeconds) {
        // See docs on set for info on options: https://redis.io/commands/set
        // We use NX to say "set if not exists" and ex to specify the expiry of this key/value
        // This is useful in implementing locks. We only use longs as values to keep things simple
        auto reply = (redisReply *) redisCommand(context, "SET %s %i NX EX %i", key.c_str(), value, expirySeconds);

        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        if (reply->type == REDIS_REPLY_ERROR) {
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

    void Redis::enqueue(const std::string &queueName, const std::vector<uint8_t> &value) {
        // NOTE: Here we must be careful with the input and specify bytes rather than a string
        // otherwise an encoded false boolean can be treated as a string terminator
        auto reply = (redisReply *) redisCommand(context, "RPUSH %s %b", queueName.c_str(), value.data(), value.size());
        freeReplyObject(reply);
    }

    std::vector<uint8_t> Redis::dequeue(const std::string &queueName, int timeout) {
        auto reply = (redisReply *) redisCommand(context, "BLPOP %s %d", queueName.c_str(), timeout);

        if (reply == nullptr || reply->type == REDIS_REPLY_NIL) {
            throw RedisNoResponseException();
        }

        size_t nResults = reply->elements;

        if (nResults > 2) {
            throw std::runtime_error("Returned more than one pair of dequeued values");
        }

        // Note, BLPOP will return the queue name and the value returned (elements 0 and 1)
        redisReply *r = reply->element[1];
        const std::vector<uint8_t> replyBytes = getBytesFromReply(r);
        freeReplyObject(reply);

        return replyBytes;
    }

    void Redis::enqueueMessage(const std::string &queueName, const message::Message &msg) {
        std::vector<uint8_t> msgBytes = infra::messageToBytes(msg);
        this->enqueue(queueName, msgBytes);
    }

    /**
     *  ------ Function messages ------
     */

    message::Message Redis::nextMessage(const std::string &queueName, int timeout) {
        std::vector<uint8_t> dequeueResult = this->dequeue(queueName, timeout);

        const auto &t = prof::startTimer();

        message::Message msg;
        msg.ParseFromArray(dequeueResult.data(), (int) dequeueResult.size());

        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->debug("Redis dequeued {}", infra::funcToString(msg));

        prof::logEndTimer("next-function", t);
        return msg;
    }

    void Redis::setFunctionResult(message::Message &msg, bool success) {
        msg.set_success(success);

        const auto &t = prof::startTimer();

        std::string key = msg.resultkey();

        // Write the successful result to the result queue
        std::vector<uint8_t> inputData = infra::messageToBytes(msg);
        this->enqueue(key, inputData);

        // Set the result key to expire
        auto reply = (redisReply *) redisCommand(context, "EXPIRE %s %d", key.c_str(), util::RESULT_KEY_EXPIRY);
        freeReplyObject(reply);

        prof::logEndTimer("function-result", t);
    }

    message::Message Redis::getFunctionResult(const message::Message &msg) {
        std::vector<uint8_t> result = this->dequeue(msg.resultkey());

        message::Message msgResult;
        msgResult.ParseFromArray(result.data(), (int) result.size());

        return msgResult;
    }
}

