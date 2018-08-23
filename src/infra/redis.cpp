#include "infra.h"
#include "util/util.h"

#include <thread>

namespace infra {

    const int BLOCKING_TIMEOUT = 1000;

    const std::string CALLS_QUEUE = "function_calls";

    Redis::Redis() {
        std::string hostname = util::getEnvVar("REDIS_HOST", "localhost");
        std::string port = util::getEnvVar("REDIS_PORT", "6379");

        std::thread::id threadId = std::this_thread::get_id();

        std::cout << "Instantiating redis in thread " << threadId << std::endl;

        int portInt = std::stoi(port);
        context = redisConnect(hostname.c_str(), portInt);
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

        // We have to be careful here to handle the bytes properly
        char *resultArray = r->str;
        int resultLen = r->len;
        std::vector<uint8_t> resultData(resultArray, resultArray + resultLen);

        return resultData;
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

