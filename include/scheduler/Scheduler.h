#pragma once

#include "InMemoryMessageQueue.h"
#include "GlobalMessageBus.h"
#include "SharingMessageBus.h"

#include <util/func.h>
#include <util/queue.h>

#include <shared_mutex>
#include <redis/Redis.h>

#define AVAILABLE_HOST_SET "available_workers"


namespace scheduler {
    // Note - default opinion when zero initialised should be maybe
    enum class SchedulerOpinion {
        MAYBE = 0,
        YES = 1,
        NO = 2,
    };

    class Scheduler {
    public:
        Scheduler();

        void callFunction(message::Message &msg, bool forceLocal=false);

        SchedulerOpinion getOpinion(const message::Message &msg);

        std::string getBestHostForFunction(const message::Message &msg);

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

        int getFunctionMaxInFlightRatio(const message::Message &msg);

        long getFunctionInFlightCount(const message::Message &msg);

        void addHostToGlobalSet(const std::string &host);

        void addHostToGlobalSet();

        void removeHostFromGlobalSet();

        void addHostToWarmSet(const std::string &funcStr);

        void removeHostFromWarmSet(const std::string &funcStr);

        void setMessageIdLogging(bool val);

        std::vector<unsigned int> getScheduledMessageIds();

    private:
        const std::string &thisHost;

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

        bool logMessageIds = false;
        std::vector<unsigned int> loggedMessageIds;
    };

    Scheduler &getScheduler();

    GlobalMessageBus &getGlobalMessageBus();
}