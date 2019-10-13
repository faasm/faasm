#include "RedisMessageBus.h"

#include <util/logging.h>
#include <util/json.h>
#include <util/timing.h>

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

    message::Message RedisMessageBus::nextMessage(int timeoutMs) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        try {
            logger->debug("Waiting for next message on {}", conf.queueName);

            if (conf.serialisation == "json") {
                std::string dequeueResult = redis.dequeue(conf.queueName, timeoutMs);
                message::Message msg = util::jsonToMessage(dequeueResult);

                return msg;
            } else {
                message::Message msg;
                std::vector<uint8_t> dequeueResult = redis.dequeueBytes(conf.queueName, timeoutMs);
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
        if (key.empty()) {
            throw std::runtime_error("Result key empty. Cannot publish result");
        }

        // Write the successful result to the result queue
        std::vector<uint8_t> inputData = util::messageToBytes(msg);
        redis.enqueueBytes(key, inputData);

        // Set the result key to expire
        redis.expire(key, util::RESULT_KEY_EXPIRY);
    }

    message::Message RedisMessageBus::getFunctionResult(unsigned int messageId, int timeoutMs) {
        if (messageId == 0) {
            throw std::runtime_error("Must provide non-zero message ID");
        }

        bool isBlocking = timeoutMs > 0;

        std::string resultKey = util::resultKeyFromMessageId(messageId);

        if (isBlocking) {
            // Blocking version will throw an exception when timing out which is handled
            // by the caller.
            std::vector<uint8_t> result = redis.dequeueBytes(resultKey, timeoutMs);
            message::Message msgResult;
            msgResult.ParseFromArray(result.data(), (int) result.size());

            return msgResult;
        } else {
            // Non-blocking version will tolerate empty responses, therefore we handle
            // the exception here
            std::vector<uint8_t> result;
            try {
                result = redis.dequeueBytes(resultKey, timeoutMs);
            } catch (redis::RedisNoResponseException &ex) {
                // Ok for no response when not blocking
            }

            message::Message msgResult;
            if (result.empty()) {
                // Empty result has special type
                msgResult.set_type(message::Message_MessageType_EMPTY);
            } else {
                // Normal response if we get something from redis
                msgResult.ParseFromArray(result.data(), (int) result.size());
            }

            return msgResult;
        }
    }

    void RedisMessageBus::requestNewWorkerNode() {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->info("Ignoring request to scale out");

        scaleOutRequests++;
    }

    int RedisMessageBus::getScaleoutRequestCount() {
        return scaleOutRequests;
    }

    void RedisMessageBus::clear() {
        scaleOutRequests = 0;
    }
}