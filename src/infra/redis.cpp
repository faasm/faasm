#include "infra.h"
#include "util/util.h"

#include <thread>

namespace infra {
    // Note - hiredis redis contexts are suitable only for single threads
    // therefore we need to ensure that each thread has its own instance
    static thread_local infra::Redis redis;

    const int BLOCKING_TIMEOUT = 1000;

    const std::string CALLS_QUEUE = "function_calls";

    Redis::Redis() {
        std::string hostname = util::getEnvVar("REDIS_HOST", "localhost");
        std::string port = util::getEnvVar("REDIS_PORT", "6379");

        int portInt = std::stoi(port);
        context = redisConnect(hostname.c_str(), portInt);
    }

    Redis *Redis::getThreadConnection() {
        return &redis;
    }

    void Redis::reconnect() {
        redisReconnect(context);
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
        return getBytesFromReply(reply);
    }

    void Redis::set(const std::string &key, const std::vector<uint8_t> &value) {
        redisCommand(context, "SET %s %b", key.c_str(), value.data(), value.size());
    }

    void Redis::setRange(const std::string &key, long offset, const std::vector<uint8_t> &value) {
        redisCommand(context, "SETRANGE %s %li %b", key.c_str(), offset, value.data(), value.size());
    }

    std::vector<uint8_t> Redis::getRange(const std::string &key, long start, long end) {
        auto reply = (redisReply *) redisCommand(context, "GETRANGE %s %li %li", key.c_str(), start, end);
        return getBytesFromReply(reply);
    }

    void Redis::enqueue(const std::string &queueName, const std::vector<uint8_t> &value) {
        // NOTE: Here we must be careful with the input and specify bytes rather than a string
        // otherwise an encoded false boolean can be treated as a string terminator
        redisCommand(context, "RPUSH %s %b", queueName.c_str(), value.data(), value.size());
    }

    std::vector<uint8_t> Redis::dequeue(const std::string &queueName) {
        auto reply = (redisReply *) redisCommand(context, "BLPOP %s %d", queueName.c_str(), BLOCKING_TIMEOUT);

        size_t nResults = reply->elements;

        if(nResults > 2) {
            throw std::runtime_error("Returned more than one pair of dequeued values");
        }

        // Note, BLPOP will return the queue name and the value returned (elements 0 and 1)
        redisReply *r = reply->element[1];
        return getBytesFromReply(r);
    }


    void Redis::flushAll() {
        redisCommand(context, "FLUSHALL");
    }

    long Redis::listLength(const std::string &queueName) {
        auto reply = (redisReply *) redisCommand(context, "LLEN %s", queueName.c_str());
        return reply->integer;
    }

    void Redis::callFunction(message::FunctionCall &call) {
        // Generate a random result key
        int randomNumber = util::randomInteger();
        std::string resultKey = "Result_";
        resultKey += std::to_string(randomNumber);
        call.set_resultkey(resultKey);

        // Send the function call
        std::cout << "Function " << call.user() << " - " << call.function() << " - " << randomNumber << std::endl;

        std::vector<uint8_t> inputData = infra::callToBytes(call);

        this->enqueue(CALLS_QUEUE, inputData);
    }

    message::FunctionCall Redis::nextFunctionCall() {
        std::vector<uint8_t> dequeueResult = this->dequeue(CALLS_QUEUE);

        message::FunctionCall call;
        call.ParseFromArray(dequeueResult.data(), (int) dequeueResult.size());

        return call;
    }

    void Redis::setFunctionResult(message::FunctionCall &call, bool success) {
        call.set_success(success);

        std::string key = call.resultkey();

        // Write the successful result to the result queue
        std::vector<uint8_t> inputData = infra::callToBytes(call);
        this->enqueue(key, inputData);
    }

    message::FunctionCall Redis::getFunctionResult(const message::FunctionCall &call) {
        std::vector<uint8_t> result = this->dequeue(call.resultkey());

        message::FunctionCall callResult;
        callResult.ParseFromArray(result.data(), (int) result.size());

        return callResult;
    }
}

