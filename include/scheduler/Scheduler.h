#pragma once

#include "InMemoryMessageQueue.h"
#include "GlobalMessageQueue.h"

#include <util/func.h>
#include <util/queue.h>

#include <shared_mutex>
#include <redis/Redis.h>

#define GLOBAL_WORKER_SET "available_workers"

namespace scheduler {
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

        std::string getHostSharingQueueName();

        GlobalMessageQueue &getGlobalQueue();
    private:
        std::string hostname;

        void addWarmThreads(const message::Message &msg);

        util::SystemConfig &conf;

        InMemoryMessageQueue *bindQueue;

        GlobalMessageQueue globalQueue;

        std::unordered_map<std::string, InMemoryMessageQueue *> queueMap;
        std::unordered_map<std::string, long> threadCountMap;
        std::shared_mutex mx;

        redis::Redis &redis;
    };

    Scheduler &getScheduler();
}