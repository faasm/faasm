#include "infra.h"
#include "util/util.h"

#include <thread>

namespace infra {

    const int BLOCKING_TIMEOUT = 1000;

    Redis::Redis() {
        std::string hostname = util::getEnvVar("REDIS_HOST", "localhost");
        std::string port = util::getEnvVar("REDIS_PORT", "6379");

        std::thread::id threadId = std::this_thread::get_id();

        std::cout << "Instantiating redis in thread " << threadId << std::endl;

        int portInt = std::stoi(port);
        context = redisConnect(hostname.c_str(), portInt);
    }

    void Redis::enqueue(const std::string &queueName, const std::string &value) {
        redisCommand(context, "RPUSH %s %s", queueName.c_str(), value.c_str());
    }

    std::string Redis::dequeue(const std::string &queueName) {
        auto reply = (redisReply *) redisCommand(context, "BLPOP %s %d", queueName.c_str(), BLOCKING_TIMEOUT);

        size_t nResults = reply->elements;

        if(nResults > 2) {
            throw std::runtime_error("Returned more than one pair of dequeued values");
        }

        // Note, BLPOP will return the queue name and the value returned (elements 0 and 1)
        redisReply *r = reply->element[1];
        return std::string(r->str);
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

        std::string serialised = call.SerializeAsString();

        this->enqueue("function_calls", serialised);
    }

    message::FunctionCall Redis::nextFunctionCall() {
        std::string queueName = "function_calls";
        std::string dequeueResult = this->dequeue(queueName);

        message::FunctionCall call;
        call.ParseFromString(dequeueResult);
        return call;
    }

    void Redis::setFunctionResult(message::FunctionCall &call, bool success) {
        call.set_success(success);

        std::string key = call.resultkey();

        // Write the successful result to the result queue
        this->enqueue(key, call.SerializeAsString());
    }

    message::FunctionCall Redis::getFunctionResult(const message::FunctionCall &call) {
        std::string result = this->dequeue(call.resultkey());

        message::FunctionCall callResult;
        callResult.ParseFromString(result);

        return callResult;
    }
}

