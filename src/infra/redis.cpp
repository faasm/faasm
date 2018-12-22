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

    // Script to atomically scale up a worker.  The logic is:
    //
    // 0. Get the current count and length of the queue
    // 1. Check if we've exceeded the ratio
    // 2a. If no workers at all, increment and return 1
    // 2b. If ratio not exceeded, return 0
    // 2c. If ratio exceeded, increment and return 1
    // 2d. If ratio exceeded and at max workers, return 0
    //
    // See Redis docs for more info: https://redis.io/commands/eval
    //
    // Arguments are: counterName, queueName, maxQueueRatio, maxWorkers
    static std::string addWorkerSha;
    static std::string addWorkerCmd = "local workerCount = redis.call(\"GET\", KEYS[1]) \n"
                                      "if not workerCount then \n"
                                      "    redis.call(\"INCR\", KEYS[1]) \n"
                                      "    return 1 \n"
                                      "end \n"
                                      ""
                                      "workerCount = tonumber(workerCount) \n"
                                      "local queueLen = redis.call(\"LLEN\", KEYS[2]) \n"
                                      "local queueRatio = queueLen / workerCount \n"
                                      "local maxQueueRatio = tonumber(ARGV[1]) \n"
                                      "local maxCount = tonumber(ARGV[2])"
                                      ""
                                      "if queueRatio >= maxQueueRatio and workerCount < maxCount then \n"
                                      "    redis.call(\"INCR\", KEYS[1]) \n"
                                      "    return 1 \n"
                                      "end \n"
                                      ""
                                      "return 0 \n";

    // Script to atomically get whether a container should prewarm. Logic is:
    //
    // 0. Get the current prewarm count
    // 1a. If it's less than the target
    static std::string incrIfBelowTargetSha;
    static std::string incrIfBelowTargetCmd = "local prewarmCount = redis.call(\"GET\", KEYS[1]) \n"
                                              "if prewarmCount then \n"
                                              "    prewarmCount = tonumber(prewarmCount) \n"
                                              "else \n"
                                              "    prewarmCount = 0 \n"
                                              "end \n"
                                              ""
                                              "local prewarmTarget = tonumber(ARGV[1]) \n"
                                              "if prewarmCount < prewarmTarget then \n"
                                              "    redis.call(\"INCR\", KEYS[1]) \n"
                                              "    return 1 \n"
                                              "end \n"
                                              ""
                                              "return 0";

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

    Redis *Redis::getThreadState() {
        return &redisState;
    }

    Redis *Redis::getThreadQueue() {
        return &redisQueue;
    }

    std::vector<uint8_t> getBytesFromReply(redisReply *reply) {
        // We have to be careful here to handle the bytes properly
        char *resultArray = reply->str;
        int resultLen = reply->len;

        std::vector<uint8_t> resultData(resultArray, resultArray + resultLen);

        return resultData;
    }

    void getBytesFromReply(redisReply *reply, uint8_t *buffer, size_t size) {
        // We have to be careful here to handle the bytes properly
        char *resultArray = reply->str;
        int resultLen = reply->len;

        if(resultLen > size) {
            throw std::runtime_error("Reading value too big for buffer");
        }

        std::copy(resultArray, resultArray + resultLen, buffer);
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

    bool Redis::addWorker(const std::string &counterName, const std::string &queueName,
                          long maxRatio, long maxWorkers) {
        // Create the script if not exists
        if (addWorkerSha.empty()) {
            const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
            addWorkerSha = this->loadScript(addWorkerCmd);
            logger->debug("Loaded worker script with SHA {}", addWorkerSha);
        };

        // Invoke the script (number 2 says how many keys there are at the start of the argument list)
        auto reply = (redisReply *) redisCommand(context, "EVALSHA %s 2 %s %s %li %li", addWorkerSha.c_str(),
                                                 counterName.c_str(), queueName.c_str(), maxRatio, maxWorkers);

        long result = extractScriptResult(reply);
        return result == 1;
    }

    bool Redis::incrIfBelowTarget(const std::string &key, int target) {
        // Create the script if not exists
        if (incrIfBelowTargetSha.empty()) {
            const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
            incrIfBelowTargetSha = this->loadScript(incrIfBelowTargetCmd);
            logger->debug("Loaded incr below target script with SHA {}", incrIfBelowTargetSha);
        };

        // Invoke the script
        auto reply = (redisReply *) redisCommand(context, "EVALSHA %s 1 %s %i", incrIfBelowTargetSha.c_str(),
                                                 key.c_str(), target);

        long result = extractScriptResult(reply);
        return result == 1;
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

    /**
     * Note that start/end are both inclusive
     */
    void Redis::getRange(const std::string &key, uint8_t *buffer, long start, long end) {
        auto reply = (redisReply *) redisCommand(context, "GETRANGE %s %li %li", key.c_str(), start, end);

        // getrange is inclusive
        long len = end - start + 1;
        getBytesFromReply(reply, buffer, len);
        freeReplyObject(reply);
    }

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

    void Redis::enqueueMessage(const std::string &queueName, const message::Message &msg) {
        std::vector<uint8_t> msgBytes = infra::messageToBytes(msg);
        this->enqueue(queueName, msgBytes);
    }

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

    long Redis::getTtl(const std::string &key) {
        auto reply = (redisReply *) redisCommand(context, "TTL %s", key.c_str());

        long ttl = reply->integer;
        freeReplyObject(reply);

        return ttl;
    }

    message::Message Redis::getFunctionResult(const message::Message &msg) {
        std::vector<uint8_t> result = this->dequeue(msg.resultkey());

        message::Message msgResult;
        msgResult.ParseFromArray(result.data(), (int) result.size());

        return msgResult;
    }

    void Redis::refresh() {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->debug("Redis reconnecting");

        redisReconnect(context);
    }
}

