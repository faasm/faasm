#include "redis.h"
#include <util/util.h>

namespace redis {
    RedisClient::RedisClient() : cpp_redis::client() {
        {
            std::string hostname = util::getEnvVar("REDIS_HOST", "localhost");

            std::cout << "Connection to Redis at " << hostname << "\n";

            this->connect(hostname, 6379);
        }
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

        this->sync_commit();

        return result;
    }

    void RedisClient::enqueue(const std::string &queueName, const std::string &value) {
        std::vector<std::string> values = {value};

        // Async call, ignore result
        this->rpush(queueName, values);
        this->commit();
    }

    void RedisClient::dequeue(const std::string &queueName, std::function<void(const std::string &)> callback) {
        this->lpop(queueName, [callback](cpp_redis::reply &reply) {
            callback(reply.as_string());
        });

        this->commit();
    }

    void RedisClient::callFunction(message::FunctionCall &call) {
        std::string serialised = call.SerializeAsString();

        this->enqueue("function_calls", serialised);
    }

    void RedisClient::nextFunctionCall(std::function<void(message::FunctionCall &)> callback) {
        std::string queueName = "function_calls";
        this->dequeue(queueName, [callback](const std::string &serialised) {
            message::FunctionCall call;
            call.ParseFromString(serialised);

            callback(call);
        });
    }
}

