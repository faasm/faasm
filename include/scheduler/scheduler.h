#pragma once

#include <shared_mutex>
#include <redis/redis.h>
#include <util/util.h>

#define GLOBAL_WORKER_SET "available_workers"

namespace scheduler {
    const std::string INCOMING_QUEUE = "incoming";

    typedef util::Queue<message::Message> InMemoryMessageQueue;
    typedef std::pair<std::string, InMemoryMessageQueue *> InMemoryMessageQueuePair;

    class MessageQueue {
        friend class Scheduler;

    public:
        MessageQueue();

        void enqueueMessage(const std::string &queueName, const message::Message &msg);

        message::Message nextMessage(const std::string &queueName, int timeout = util::DEFAULT_TIMEOUT);

        void setFunctionResult(message::Message &msg, bool success);

        message::Message getFunctionResult(const message::Message &msg);

    private:
        redis::Redis &redis;
    };

    class Scheduler {
    public:
        Scheduler();

        ~Scheduler();

        const int scheduleWaitMillis = 100;

        const int scheduleRecursionLimit = 10;

        void addCurrentHostToWorkerPool();

        void placeOnGlobalQueue(message::Message &msg);

        MessageQueue &getGlobalQueue();

        void callFunction(message::Message &msg);

        std::string getBestHostForFunction(const message::Message &msg);

        void enqueueMessage(const message::Message &msg);

        InMemoryMessageQueue *listenToQueue(const message::Message &msg);

        InMemoryMessageQueue *getFunctionQueue(const message::Message &msg);

        void stopListeningToQueue(const message::Message &msg);

        InMemoryMessageQueue *getBindQueue();

        std::string getFunctionWarmSetName(const message::Message &msg);

        void clear();

    private:
        std::string hostname;

        void addWarmThreads(const message::Message &msg);

        util::SystemConfig &conf;

        MessageQueue messageQueue;
        InMemoryMessageQueue *bindQueue;

        std::map<std::string, InMemoryMessageQueue *> queueMap;
        std::map<std::string, long> threadCountMap;
        std::shared_mutex mx;

        redis::Redis &redis;

        long getFunctionThreadCount(const message::Message &msg);

        double getFunctionQueueRatio(const message::Message &msg);

        long getFunctionQueueLength(const message::Message &msg);
    };

    Scheduler &getScheduler();
}