#pragma once

#include "InMemoryMessageQueue.h"
#include "GlobalMessageBus.h"
#include "SharingMessageBus.h"

#include <util/func.h>
#include <util/queue.h>

#include <shared_mutex>
#include <redis/Redis.h>

#define GLOBAL_WORKER_SET "available_workers"

namespace scheduler {
    class Scheduler {
    public:
        Scheduler();

        void callFunction(message::Message &msg);

        std::string getBestHostForFunction(const message::Message &msg);

        void enqueueMessage(const message::Message &msg);

        std::shared_ptr<InMemoryMessageQueue> listenToQueue(const message::Message &msg);

        std::shared_ptr<InMemoryMessageQueue> getFunctionQueue(const message::Message &msg);

        void stopListeningToQueue(const message::Message &msg);

        std::shared_ptr<InMemoryMessageQueue> getBindQueue();

        std::string getFunctionWarmSetName(const message::Message &msg);

        std::string getFunctionWarmSetNameFromStr(const std::string &funcStr);

        void clear();

        long getFunctionThreadCount(const message::Message &msg);

        double getFunctionQueueRatio(const message::Message &msg);

        long getFunctionQueueLength(const message::Message &msg);

        void addHostToGlobalSet();

        long getGlobalSetSize();

        void addHostToWarmSet(const std::string &funcStr);

        void removeHostFromWarmSet(const std::string &funcStr);

        int getExecutingCount();

        void incrementExecutingCount();

        void decrementExecutingCount();

        void scaleOut(int targetCount);
    private:
        std::string hostname;

        void removeHostFromGlobalSet();

        void addWarmThreads(const message::Message &msg);

        util::SystemConfig &conf;

        std::shared_ptr<InMemoryMessageQueue> bindQueue;

        std::unordered_map<std::string, std::shared_ptr<InMemoryMessageQueue>> queueMap;
        std::unordered_map<std::string, long> threadCountMap;
        std::shared_mutex mx;

        std::shared_mutex execCountMutex;
        int execCount = 0;

        SharingMessageBus &sharingBus;
    };

    Scheduler &getScheduler();

    GlobalMessageBus &getGlobalMessageBus();
}