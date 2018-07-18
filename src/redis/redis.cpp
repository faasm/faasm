#include "redis.h"
#include <util/util.h>

namespace redis {
    RedisClient::RedisClient() : cpp_redis::client() {
        {

        }
    }

    void RedisClient::connect() {
        std::string hostname = util::getEnvVar("REDIS_HOST", "localhost");

        std::cout << "Connection to Redis at " << hostname << "\n";

        cpp_redis::client::connect(hostname, 6379);
    }

    std::string RedisClient::check(const std::string &value) {
        const char *key = "check_key";
        std::string result;

        this->set(key, value, [](cpp_redis::reply &reply) {
            // Do nothing
        });

        this->get(key, [&result](cpp_redis::reply &reply) {
            result = reply.as_string();
        });

        return result;
    }

    void RedisClient::enqueue(const std::string &queueName, const std::string &value) {
        std::vector<std::string> values = {value};

        // Async call, ignore result
        this->rpush(queueName, values);
        this->sync_commit();
    }

    /**
     * Note that this function is blocking
     */
    std::vector<std::string> RedisClient::blockingDequeue(const std::string &queueName) {
        // Note, blpop takes a list of keys on which to block. It then returns an array
        // with the key that yielded a result along with the result itself.
        // This means we need to do a bit of fiddling with arrays.
        std::future<cpp_redis::reply> popFuture = this->blpop({queueName}, 0);
        this->commit();

        cpp_redis::reply popReply = popFuture.get();

        const std::vector<cpp_redis::reply> &result = popReply.as_array();

        const std::string &keyName = result[0].as_string();
        const std::string &value = result[1].as_string();

        return {keyName, value};
    }

    void RedisClient::callFunction(const message::FunctionCall &call) {
        std::string serialised = call.SerializeAsString();

        this->enqueue("function_calls", serialised);
    }

    /**
     * Note that this function is blocking
     */
    message::FunctionCall RedisClient::nextFunctionCall() {
        std::string queueName = "function_calls";
        std::vector<std::string> dequeueResult = this->blockingDequeue(queueName);

        message::FunctionCall call;
        call.ParseFromString(dequeueResult[1]);
        return call;
    }

    void RedisClient::setFunctionResult(message::FunctionCall &call, bool success) {
        call.set_success(success);

        std::string key = call.resultkey();

        // Write the successful result
        this->set(key, call.SerializeAsString());
        this->commit();
    }

    message::FunctionCall RedisClient::blockingGetFunctionResult(const message::FunctionCall &call) {
        std::future<cpp_redis::reply> getFuture = this->get(call.resultkey());
        this->commit();

        cpp_redis::reply getReply = getFuture.get();
        const std::string &serialised = getReply.as_string();

        message::FunctionCall result;
        result.ParseFromString(serialised);
        return result;
    };
}

