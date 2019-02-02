#pragma once

#include <util/func.h>
#include <util/queue.h>

#include <shared_mutex>
#include <redis/Redis.h>

#define GLOBAL_WORKER_SET "available_workers"

namespace scheduler {
    typedef util::Queue<message::Message> InMemoryMessageQueue;
    typedef std::pair<std::string, InMemoryMessageQueue *> InMemoryMessageQueuePair;

    class MessageQueue {
        friend class Scheduler;

    public:
        static MessageQueue &getGlobalQueue();

        explicit MessageQueue(const std::string &queueName);

        void enqueueMessage(const message::Message &msg);

        message::Message nextMessage(int timeout = util::DEFAULT_TIMEOUT);

        void setFunctionResult(message::Message &msg, bool success);

        message::Message getFunctionResult(const message::Message &msg);

    private:
        std::string queueName;
        redis::Redis &redis;
    };
}