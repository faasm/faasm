#pragma once

#include "InMemoryMessageQueue.h"
#include "GlobalMessageBus.h"
#include "SharingMessageBus.h"

#include <util/func.h>
#include <util/queue.h>

#include <shared_mutex>
#include <redis/Redis.h>

#define GLOBAL_NODE_SET "available_workers"

// Make this long enough for big batches
#define CHAINED_CALL_TIMEOUT_MS 300000


namespace scheduler {
    // Note - default opinion should be maybe
    enum SchedulerOpinion {
        MAYBE,
        YES,
        NO
    };

    class Scheduler {
    public:
        Scheduler();

        void callFunction(message::Message &msg, bool forceLocal=false);

        SchedulerOpinion getOpinion(const message::Message &msg);

        std::string getBestNodeForFunction(const message::Message &msg);

        void enqueueMessage(const message::Message &msg);

        std::shared_ptr<InMemoryMessageQueue> getFunctionQueue(const message::Message &msg);

        void notifyCallFinished(const message::Message &msg);

        void notifyThreadFinished(const message::Message &msg);

        void notifyAwaiting(const message::Message &msg);

        void notifyFinishedAwaiting(const message::Message &msg);

        std::shared_ptr<InMemoryMessageQueue> getBindQueue();

        std::string getFunctionWarmSetName(const message::Message &msg);

        std::string getFunctionWarmSetNameFromStr(const std::string &funcStr);

        void clear();

        long getFunctionThreadCount(const message::Message &msg);

        double getFunctionInFlightRatio(const message::Message &msg);

        long getFunctionInFlightCount(const message::Message &msg);

        void addNodeToGlobalSet(const std::string &node);

        void addNodeToGlobalSet();

        void removeNodeFromGlobalSet();

        void addNodeToWarmSet(const std::string &funcStr);

        void removeNodeFromWarmSet(const std::string &funcStr);

    private:
        std::string nodeId;

        void updateOpinion(const message::Message &msg);

        void addWarmThreads(const message::Message &msg);

        util::SystemConfig &conf;

        std::shared_ptr<InMemoryMessageQueue> bindQueue;

        std::shared_mutex mx;
        std::unordered_map<std::string, std::shared_ptr<InMemoryMessageQueue>> queueMap;
        std::unordered_map<std::string, long> threadCountMap;
        std::unordered_map<std::string, long> inFlightCountMap;
        std::unordered_map<std::string, SchedulerOpinion> opinionMap;

        SharingMessageBus &sharingBus;
    };

    Scheduler &getScheduler();

    GlobalMessageBus &getGlobalMessageBus();
}