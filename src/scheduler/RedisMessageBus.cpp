#include "RedisMessageBus.h"

#include <util/logging.h>
#include <util/json.h>
#include <util/timing.h>
#include <util/bytes.h>

#define CHAINED_SET_PREFIX "chained_"

namespace scheduler {
    std::string getChainedKey(unsigned int msgId) {
        return std::string(CHAINED_SET_PREFIX) + std::to_string(msgId);
    }

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
        return this->nextMessage(DEFAULT_TIMEOUT);
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
            throw GlobalMessageBusNoMessageException("No message from global bus");
        }
    }

    void RedisMessageBus::setFunctionResult(message::Message &msg) {
        // Record which host did the execution
        msg.set_executedhost(util::getSystemConfig().endpointHost);

        std::string key = msg.resultkey();
        if (key.empty()) {
            throw std::runtime_error("Result key empty. Cannot publish result");
        }

        // Write the successful result to the result queue
        std::vector<uint8_t> inputData = util::messageToBytes(msg);
        redis.enqueueBytes(key, inputData);

        // Set the result key to expire
        redis.expire(key, RESULT_KEY_EXPIRY);

        // Set long-lived result for function too
        if (conf.execGraphMode == "on") {
            redis.set(msg.statuskey(), inputData);
        }
    }

    message::Message RedisMessageBus::getFunctionResult(unsigned int messageId, int timeoutMs) {
        if (messageId == 0) {
            throw std::runtime_error("Must provide non-zero message ID");
        }

        bool isBlocking = timeoutMs > 0;

        std::string resultKey = util::resultKeyFromMessageId(messageId);

        message::Message msgResult;

        if (isBlocking) {
            // Blocking version will throw an exception when timing out which is handled
            // by the caller.
            std::vector<uint8_t> result = redis.dequeueBytes(resultKey, timeoutMs);
            msgResult.ParseFromArray(result.data(), (int) result.size());
        } else {
            // Non-blocking version will tolerate empty responses, therefore we handle
            // the exception here
            std::vector<uint8_t> result;
            try {
                result = redis.dequeueBytes(resultKey, timeoutMs);
            } catch (redis::RedisNoResponseException &ex) {
                // Ok for no response when not blocking
            }

            if (result.empty()) {
                // Empty result has special type
                msgResult.set_type(message::Message_MessageType_EMPTY);
            } else {
                // Normal response if we get something from redis
                msgResult.ParseFromArray(result.data(), (int) result.size());
            }
        }

        return msgResult;
    }


    std::string RedisMessageBus::getMessageStatus(unsigned int messageId) {
        const message::Message result = getFunctionResult(messageId, 0);

        if (result.type() == message::Message_MessageType_EMPTY) {
            return "RUNNING";
        } else if (result.returnvalue() == 0) {
            return "SUCCESS: " + result.outputdata();
        } else {
            return "FAILED: " + result.outputdata();
        }
    }

    void RedisMessageBus::logChainedFunction(unsigned int parentMessageId, unsigned int chainedMessageId) {
        const std::string &key = getChainedKey(parentMessageId);
        redis.sadd(key, std::to_string(chainedMessageId));
    }

    std::vector<unsigned int> RedisMessageBus::getChainedFunctions(unsigned int msgId) {
        const std::string &key = getChainedKey(msgId);
        const std::unordered_set<std::string> chainedCalls = redis.smembers(key);

        std::vector<unsigned int> chainedIds;
        for(auto i : chainedCalls) {
            chainedIds.emplace_back(std::stoi(i));
        }

        return chainedIds;
    }

    void RedisMessageBus::clear() {
    }

}