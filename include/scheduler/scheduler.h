#pragma once

#include <util/func.h>
#include <util/queue.h>

#include <shared_mutex>
#include <redis/redis.h>

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

    class Scheduler {
    public:
        Scheduler();

        ~Scheduler();

        void callFunction(message::Message &msg);

        std::string getBestHostForFunction(const message::Message &msg);

        void enqueueMessage(const message::Message &msg);

        InMemoryMessageQueue *listenToQueue(const message::Message &msg);

        InMemoryMessageQueue *getFunctionQueue(const message::Message &msg);

        void stopListeningToQueue(const message::Message &msg);

        InMemoryMessageQueue *getBindQueue();

        std::string getFunctionWarmSetName(const message::Message &msg);

        void clear();

        long getFunctionThreadCount(const message::Message &msg);

        double getFunctionQueueRatio(const message::Message &msg);

        long getFunctionQueueLength(const message::Message &msg);

    private:
        std::string hostname;

        void addWarmThreads(const message::Message &msg);

        util::SystemConfig &conf;

        InMemoryMessageQueue *bindQueue;

        std::map<std::string, InMemoryMessageQueue *> queueMap;
        std::map<std::string, long> threadCountMap;
        std::shared_mutex mx;

        redis::Redis &redis;
    };

    Scheduler &getScheduler();
}