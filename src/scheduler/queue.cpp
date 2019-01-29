#include "scheduler.h"

namespace scheduler {
    /**
     *  ------ Function messages ------
     */

    MessageQueue::MessageQueue() : redis(redis::Redis::getQueue()) {

    }

    void MessageQueue::enqueueMessage(const std::string &queueName, const message::Message &msg) {
        std::vector<uint8_t> msgBytes = util::messageToBytes(msg);
        redis.enqueue(queueName, msgBytes);
    }


    message::Message MessageQueue::nextMessage(const std::string &queueName, int timeout) {
        std::vector<uint8_t> dequeueResult = redis.dequeue(queueName, timeout);

        message::Message msg;
        msg.ParseFromArray(dequeueResult.data(), (int) dequeueResult.size());

        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->debug("MessageQueue dequeued {}", util::funcToString(msg));

        return msg;
    }

    void MessageQueue::setFunctionResult(message::Message &msg, bool success) {
        msg.set_success(success);

        std::string key = msg.resultkey();

        // Write the successful result to the result queue
        std::vector<uint8_t> inputData = util::messageToBytes(msg);
        redis.enqueue(key, inputData);

        // Set the result key to expire
        redis.expire(key, util::RESULT_KEY_EXPIRY);
    }

    message::Message MessageQueue::getFunctionResult(const message::Message &msg) {
        std::vector<uint8_t> result = redis.dequeue(msg.resultkey());

        message::Message msgResult;
        msgResult.ParseFromArray(result.data(), (int) result.size());

        return msgResult;
    }
}