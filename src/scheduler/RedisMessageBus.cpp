#include "RedisMessageBus.h"

#include <util/logging.h>

namespace scheduler {
    RedisMessageBus::RedisMessageBus(const std::string &queueNameIn) : GlobalMessageBus(queueNameIn),
                                                                       redis(redis::Redis::getQueue()) {

    }

    void RedisMessageBus::enqueueMessage(const message::Message &msg) {
        std::vector<uint8_t> msgBytes = util::messageToBytes(msg);
        redis.enqueue(queueName, msgBytes);
    }

    message::Message RedisMessageBus::nextMessage() {
        return this->nextMessage(util::DEFAULT_TIMEOUT);
    }

    message::Message RedisMessageBus::nextMessage(int timeout) {
        std::vector<uint8_t> dequeueResult = redis.dequeue(queueName, timeout);

        message::Message msg;
        msg.ParseFromArray(dequeueResult.data(), (int) dequeueResult.size());

        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->debug("Message queue dequeued {}", util::funcToString(msg));

        return msg;
    }

    void RedisMessageBus::setFunctionResult(message::Message &msg, bool success) {
        msg.set_success(success);

        std::string key = msg.resultkey();

        // Write the successful result to the result queue
        std::vector<uint8_t> inputData = util::messageToBytes(msg);
        redis.enqueue(key, inputData);

        // Set the result key to expire
        redis.expire(key, util::RESULT_KEY_EXPIRY);
    }

    message::Message RedisMessageBus::getFunctionResult(const message::Message &msg) {
        std::vector<uint8_t> result = redis.dequeue(msg.resultkey());

        message::Message msgResult;
        msgResult.ParseFromArray(result.data(), (int) result.size());

        return msgResult;
    }
}