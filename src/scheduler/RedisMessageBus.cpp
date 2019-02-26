#include "RedisMessageBus.h"

#include <util/logging.h>
#include <util/json.h>

namespace scheduler {
    RedisMessageBus::RedisMessageBus() : redis(redis::Redis::getQueue()) {

    }

    void RedisMessageBus::enqueueMessage(const message::Message &msg) {
        if (conf.serialisation == "json") {
            const std::string json = util::messageToJson(msg);
            redis.enqueue(conf.queueName, json);
        } else {
            std::vector<uint8_t> msgBytes = util::messageToBytes(msg);
            redis.enqueueBytes(conf.queueName, msgBytes);
        }
    }

    message::Message RedisMessageBus::nextMessage() {
        return this->nextMessage(util::DEFAULT_TIMEOUT);
    }

    message::Message RedisMessageBus::nextMessage(int timeout) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        try {
            logger->debug("Waiting for next message on {}", conf.queueName);

            if (conf.serialisation == "json") {
                std::string dequeueResult = redis.dequeue(conf.queueName);
                message::Message msg = util::jsonToMessage(dequeueResult);

                return msg;
            } else {
                message::Message msg;
                std::vector<uint8_t> dequeueResult = redis.dequeueBytes(conf.queueName, timeout);
                msg.ParseFromArray(dequeueResult.data(), (int) dequeueResult.size());

                return msg;
            }
        }
        catch (redis::RedisNoResponseException &ex) {
            throw GlobalMessageBusNoMessageException();
        }
    }

    void RedisMessageBus::setFunctionResult(message::Message &msg, bool success) {
        msg.set_success(success);

        std::string key = msg.resultkey();
        if(key.empty()) {
            throw std::runtime_error("Result key empty. Cannot publish result");
        }

        // Write the successful result to the result queue
        std::vector<uint8_t> inputData = util::messageToBytes(msg);
        redis.enqueueBytes(key, inputData);

        // Set the result key to expire
        redis.expire(key, util::RESULT_KEY_EXPIRY);
    }

    message::Message RedisMessageBus::getFunctionResult(const message::Message &msg) {
        std::vector<uint8_t> result = redis.dequeueBytes(msg.resultkey());

        message::Message msgResult;
        msgResult.ParseFromArray(result.data(), (int) result.size());

        return msgResult;
    }
}