#include "infra.h"
#include "util/util.h"
#include "prof/prof.h"

#include <thread>

namespace infra {
    // Note - hiredis redis contexts are suitable only for single threads
    // therefore we need to ensure that each thread has its own instance
    static thread_local infra::Redis redis;

    // Once we have resolved the IP of the redis instance, we need to keep using it
    // This allows things operating within the network namespace to resolve it properly
    static std::string redisIp = "not_set";

    Redis::Redis() {
        hostname = util::getEnvVar("REDIS_HOST", "localhost");
        port = util::getEnvVar("REDIS_PORT", "6379");

        if (redisIp == "not_set") {
            redisIp = util::getIPFromHostname(hostname);
        }

        // Note, connect with IP, not with hostname
        int portInt = std::stoi(port);

        context = redisConnect(redisIp.c_str(), portInt);
    }

    Redis::~Redis() {
        redisFree(context);
    }

    Redis *Redis::getThreadConnection() {
        return &redis;
    }

    std::vector<uint8_t> getBytesFromReply(redisReply *reply) {
        // We have to be careful here to handle the bytes properly
        char *resultArray = reply->str;
        int resultLen = reply->len;

        std::vector<uint8_t> resultData(resultArray, resultArray + resultLen);

        return resultData;
    }

    std::vector<uint8_t> Redis::get(const std::string &key) {
        auto reply = (redisReply *) redisCommand(context, "GET %s", key.c_str());

        const std::vector<uint8_t> replyBytes = getBytesFromReply(reply);
        freeReplyObject(reply);

        return replyBytes;
    }

    bool Redis::sismember(const std::string &key, const std::string &value) {
        auto reply = (redisReply *) redisCommand(context, "SISMEMBER %s %s", key.c_str(), value.c_str());

        bool isMember = reply->integer > 0;

        freeReplyObject(reply);

        return isMember;
    }

    long Redis::scard(const std::string &key) {
        auto reply = (redisReply *) redisCommand(context, "SCARD %s", key.c_str());

        long count = reply->integer;
        freeReplyObject(reply);

        return count;
    }

    void Redis::sadd(const std::string &key, const std::string &value) {
        auto reply = (redisReply *) redisCommand(context, "SADD %s %s", key.c_str(), value.c_str());
        freeReplyObject(reply);
    }

    void Redis::srem(const std::string &key, const std::string &value) {
        auto reply = (redisReply *) redisCommand(context, "SREM %s %s", key.c_str(), value.c_str());
        freeReplyObject(reply);
    }

    std::string Redis::spop(const std::string &key) {
        auto reply = (redisReply *) redisCommand(context, "SPOP %s", key.c_str());

        std::string result;
        if (reply->type == REDIS_REPLY_NIL) {
            result = "";
        }
        else {
            result = reply-> str;
        }

        freeReplyObject(reply);
        return result;
    }

    void Redis::set(const std::string &key, const std::vector<uint8_t> &value) {
        auto reply = (redisReply *) redisCommand(context, "SET %s %b", key.c_str(), value.data(), value.size());

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

    void Redis::setRange(const std::string &key, long offset, const std::vector<uint8_t> &value) {
        auto reply = (redisReply *) redisCommand(context, "SETRANGE %s %li %b", key.c_str(), offset, value.data(), value.size());
        freeReplyObject(reply);
    }

    std::vector<uint8_t> Redis::getRange(const std::string &key, long start, long end) {
        auto reply = (redisReply *) redisCommand(context, "GETRANGE %s %li %li", key.c_str(), start, end);

        const std::vector<uint8_t> replyBytes = getBytesFromReply(reply);
        freeReplyObject(reply);

        return replyBytes;
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
        long result = reply->integer;
        freeReplyObject(reply);

        return result;
    }

    std::string getFunctionQueueName(const message::Message &msg) {
        std::string funcQueueName = infra::funcToString(msg);

        return funcQueueName;
    }

    std::string getFunctionSetName(const message::Message &msg) {
        std::string funcSetName = SET_PREFIX + infra::funcToString(msg);

        return funcSetName;
    }

    void addResultKeyToMessage(message::Message &msg) {
        // Generate a random result key
        int randomNumber = util::randomInteger();
        std::string resultKey = "Result_";
        resultKey += std::to_string(randomNumber);
        msg.set_resultkey(resultKey);
    }

    void Redis::enqueueMessage(const std::string &queueName, const message::Message &msg) {
        std::vector<uint8_t> msgBytes = infra::messageToBytes(msg);
        this->enqueue(queueName, msgBytes);
    }

    void Redis::callFunction(message::Message &msg) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        const auto &t = prof::startTimer();

        // First of all, send the message to execute the function
        const std::string queueName = getFunctionQueueName(msg);
        logger->debug("Adding call {} to {}", infra::funcToString(msg), queueName);
        addResultKeyToMessage(msg);
        this->enqueueMessage(queueName, msg);

        // Then add more workers if necessary
        this->addMoreWorkers(msg, queueName);

        prof::logEndTimer("call-function", t);
    }

    void Redis::addMoreWorkers(message::Message &msg, const std::string &queueName) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        util::SystemConfig conf = util::getSystemConfig();

        // Get queue length and set membership
        const std::string setName = getFunctionSetName(msg);

        long queueLength = this->listLength(queueName);
        long funcSetSize = this->scard(setName);

        // Check whether we need more workers
        bool needsMoreWorkerThreads;
        if(funcSetSize == 0) {
            needsMoreWorkerThreads = true;
        }
        else {
            double queueRatio = double(queueLength) / funcSetSize;
            needsMoreWorkerThreads = queueRatio > conf.max_queue_ratio && funcSetSize < conf.max_workers_per_function;
        }

        // Send bind message to pre-warm queue to enlist help of other workers
        if(needsMoreWorkerThreads) {
            logger->debug("Requesting more workers for {}", infra::funcToString(msg));

            // Ask a prewarm worker to bind to the function
            message::Message bindMsg = infra::buildBindMessage(msg, funcSetSize + 1);
            this->enqueueMessage(PREWARM_QUEUE, bindMsg);
        }
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

