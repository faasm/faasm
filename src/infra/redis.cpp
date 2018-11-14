#include "infra.h"
#include "util/util.h"

#include <thread>

namespace infra {
    // Note - hiredis redis contexts are suitable only for single threads
    // therefore we need to ensure that each thread has its own instance
    static thread_local infra::Redis redis;

    // Once we have resolved the IP of the redis instance, we need to keep using it
    // This allows things operating within the network namespace to resolve it properly
    static std::string redisIp = "not_set";

    static const int BLOCKING_TIMEOUT_SECONDS = 60;
    static const int RESULT_KEY_EXPIRY_SECONDS = 30;

    static const std::string CALLS_QUEUE = "function_calls";

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

    void Redis::set(const std::string &key, const std::vector<uint8_t> &value) {
        auto reply = (redisReply *) redisCommand(context, "SET %s %b", key.c_str(), value.data(), value.size());

        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        if (reply->type == REDIS_REPLY_ERROR) {
            logger->error("Failed to SET {} - {}", key.c_str(), reply->str);
        }

        freeReplyObject(reply);
    }

    void Redis::del(const std::string &key) {
        redisCommand(context, "DEL %s", key.c_str());
    }

    void Redis::setRange(const std::string &key, long offset, const std::vector<uint8_t> &value) {
        redisCommand(context, "SETRANGE %s %li %b", key.c_str(), offset, value.data(), value.size());
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
        redisCommand(context, "RPUSH %s %b", queueName.c_str(), value.data(), value.size());
    }

    std::vector<uint8_t> Redis::dequeue(const std::string &queueName) {
        auto reply = (redisReply *) redisCommand(context, "BLPOP %s %d", queueName.c_str(), BLOCKING_TIMEOUT_SECONDS);

        if (reply == nullptr || reply->type == REDIS_REPLY_NIL) {
            const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
            logger->debug("No response from Redis");
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
        redisCommand(context, "FLUSHALL");
    }

    long Redis::listLength(const std::string &queueName) {
        auto reply = (redisReply *) redisCommand(context, "LLEN %s", queueName.c_str());
        long result = reply->integer;
        freeReplyObject(reply);

        return result;
    }

    void Redis::callFunction(message::FunctionCall &call) {
        // Generate a random result key
        int randomNumber = util::randomInteger();
        std::string resultKey = "Result_";
        resultKey += std::to_string(randomNumber);
        call.set_resultkey(resultKey);

        std::vector<uint8_t> inputData = infra::callToBytes(call);

        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->debug("Redis enqueued ({}/{})", call.user(), call.function());

        this->enqueue(CALLS_QUEUE, inputData);
    }

    message::FunctionCall Redis::nextFunctionCall() {
        std::vector<uint8_t> dequeueResult = this->dequeue(CALLS_QUEUE);

        message::FunctionCall call;
        call.ParseFromArray(dequeueResult.data(), (int) dequeueResult.size());

        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->debug("Redis dequeued ({}/{})", call.user(), call.function());
        return call;
    }

    void Redis::setFunctionResult(message::FunctionCall &call, bool success) {
        call.set_success(success);

        std::string key = call.resultkey();

        // Write the successful result to the result queue
        std::vector<uint8_t> inputData = infra::callToBytes(call);
        this->enqueue(key, inputData);

        // Set the result key to expire
        redisCommand(context, "EXPIRE %s %d", key.c_str(), RESULT_KEY_EXPIRY_SECONDS);
    }

    long Redis::getTtl(const std::string &key) {
        auto reply = (redisReply *) redisCommand(context, "TTL %s", key.c_str());

        long ttl = reply->integer;
        return ttl;
    }

    message::FunctionCall Redis::getFunctionResult(const message::FunctionCall &call) {
        std::vector<uint8_t> result = this->dequeue(call.resultkey());

        message::FunctionCall callResult;
        callResult.ParseFromArray(result.data(), (int) result.size());

        return callResult;
    }

    void Redis::refresh() {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->debug("Redis reconnecting");

        redisReconnect(context);
    }
}

