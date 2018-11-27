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

    static const int RESULT_KEY_EXPIRY_SECONDS = 30;

    // Parameters for scheduling
    static const double MAX_QUEUE_RATIO = 5.0;
    static const int MAX_SET_SIZE = 20;

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

    std::string getFunctionQueueName(const message::Message &call) {
        std::string funcQueueName = infra::funcToString(call);

        return funcQueueName;
    }

    std::string getFunctionSetName(const message::Message &call) {
        std::string funcSetName = "f_" + infra::funcToString(call);

        return funcSetName;
    }

    void addResultKeyToMessage(message::Message &msg) {
        // Generate a random result key
        int randomNumber = util::randomInteger();
        std::string resultKey = "Result_";
        resultKey += std::to_string(randomNumber);
        msg.set_resultkey(resultKey);
    }

    void Redis::callFunction(message::Message &call) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        const auto &t = prof::startTimer();

        // Get queue length and set membership
        const std::string setName = getFunctionSetName(call);
        const std::string queueName = getFunctionQueueName(call);
        long queueLength = this->listLength(queueName);
        long setSize = this->scard(setName);

        double queueRatio = double(queueLength) / setSize;
        if(queueRatio > MAX_QUEUE_RATIO && setSize < MAX_SET_SIZE) {
            // Send bind message to pre-warm queue
            message::Message bindMsg;
            bindMsg.set_type(message::Message_MessageType_BIND);
            bindMsg.set_user(call.user());
            bindMsg.set_function(call.function());

            logger->debug("Requesting prewarm bind for {}", infra::funcToString(call));
            std::vector<uint8_t> bindData = infra::messageToBytes(bindMsg);
            this->enqueue(PREWARM_QUEUE, bindData);
        }

        // Add the call for the function
        logger->debug("Adding call {} to {}", infra::funcToString(call), queueName);
        addResultKeyToMessage(call);
        std::vector<uint8_t> callData = infra::messageToBytes(call);
        this->enqueue(queueName, callData);

        prof::logEndTimer("call-function", t);
    }

    message::Message Redis::nextMessage(const std::string &queueName, int timeout) {
        std::vector<uint8_t> dequeueResult = this->dequeue(queueName, timeout);

        const auto &t = prof::startTimer();

        message::Message call;
        call.ParseFromArray(dequeueResult.data(), (int) dequeueResult.size());

        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->debug("Redis dequeued {}", infra::funcToString(call));

        prof::logEndTimer("next-function", t);
        return call;
    }

    void Redis::setFunctionResult(message::Message &call, bool success) {
        call.set_success(success);

        const auto &t = prof::startTimer();

        std::string key = call.resultkey();

        // Write the successful result to the result queue
        std::vector<uint8_t> inputData = infra::messageToBytes(call);
        this->enqueue(key, inputData);

        // Set the result key to expire
        auto reply = (redisReply *) redisCommand(context, "EXPIRE %s %d", key.c_str(), RESULT_KEY_EXPIRY_SECONDS);
        freeReplyObject(reply);

        prof::logEndTimer("function-result", t);
    }

    long Redis::getTtl(const std::string &key) {
        auto reply = (redisReply *) redisCommand(context, "TTL %s", key.c_str());

        long ttl = reply->integer;
        freeReplyObject(reply);

        return ttl;
    }

    message::Message Redis::getFunctionResult(const message::Message &call) {
        std::vector<uint8_t> result = this->dequeue(call.resultkey());

        message::Message callResult;
        callResult.ParseFromArray(result.data(), (int) result.size());

        return callResult;
    }

    void Redis::refresh() {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->debug("Redis reconnecting");

        redisReconnect(context);
    }
}

