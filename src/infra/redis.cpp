#include "infra.h"
#include "util/util.h"

namespace infra {

    Redis::Redis() {

    }

    void Redis::connect() {
        std::string hostname = util::getEnvVar("REDIS_HOST", "localhost");

        context = redisConnect(hostname.c_str(), 6379);
    }

    void Redis::enqueue(const std::string &queueName, const std::string &value) {
        redisCommand(context, "RPUSH %s %s", queueName.c_str(), value.c_str());
    }

    std::string Redis::dequeue(const std::string &queueName) {
        auto reply = (redisReply *) redisCommand(context, "LPOP %s", queueName.c_str());
        return std::string(reply->str);
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
        std::cout << "Calling function " << call.user() << " - " << call.function() << " - " << randomNumber << "\n";

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

