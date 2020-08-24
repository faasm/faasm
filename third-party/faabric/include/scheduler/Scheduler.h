#pragma once

#include "InMemoryMessageQueue.h"
#include "ExecGraph.h"

#include <util/func.h>
#include <util/config.h>
#include <util/queue.h>

#include <shared_mutex>

#define AVAILABLE_HOST_SET "available_faaslets"

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

        void callFunction(faabric::Message &msg, bool forceLocal=false);

        SchedulerOpinion getLatestOpinion(const faabric::Message &msg);

        std::string getBestHostForFunction(const faabric::Message &msg);

        void enqueueMessage(const faabric::Message &msg);

        std::shared_ptr<InMemoryMessageQueue> getFunctionQueue(const faabric::Message &msg);

        void notifyCallFinished(const faabric::Message &msg);

        void notifyFaasletFinished(const faabric::Message &msg);

        std::shared_ptr<InMemoryMessageQueue> getBindQueue();

        std::string getFunctionWarmSetName(const faabric::Message &msg);

        std::string getFunctionWarmSetNameFromStr(const std::string &funcStr);

        void clear();

        long getFunctionWarmFaasletCount(const faabric::Message &msg);

        long getTotalWarmFaasletCount();

        double getFunctionInFlightRatio(const faabric::Message &msg);

        long getFunctionInFlightCount(const faabric::Message &msg);

        void addHostToGlobalSet(const std::string &host);

        void addHostToGlobalSet();

        void removeHostFromGlobalSet();

        void addHostToWarmSet(const std::string &funcStr);

        void removeHostFromWarmSet(const std::string &funcStr);

        void setTestMode(bool val);

        std::vector<unsigned int> getRecordedMessagesAll();

        std::vector<unsigned int> getRecordedMessagesLocal();

        std::vector<std::pair<std::string, unsigned int>> getRecordedMessagesShared();

        bool hasHostCapacity();

        std::string getThisHost();

        void broadcastFlush(const faabric::Message &msg);

        void preflightPythonCall();

        std::string getMessageStatus(unsigned int messageId);

        void setFunctionResult(faabric::Message &msg);

        faabric::Message getFunctionResult(unsigned int messageId, int timeout);

        void logChainedFunction(unsigned int parentMessageId, unsigned int chainedMessageId);

        std::unordered_set<unsigned int> getChainedFunctions(unsigned int msgId);

        ExecGraph getFunctionExecGraph(unsigned int msgId);
    private:
        std::string thisHost;

        util::SystemConfig &conf;

        std::shared_ptr<InMemoryMessageQueue> bindQueue;

        std::shared_mutex mx;
        std::unordered_map<std::string, std::shared_ptr<InMemoryMessageQueue>> queueMap;
        std::unordered_map<std::string, long> faasletCountMap;
        std::unordered_map<std::string, long> inFlightCountMap;
        std::unordered_map<std::string, SchedulerOpinion> opinionMap;
        bool _hasHostCapacity = true;

        bool isTestMode = false;
        std::vector<unsigned int> recordedMessagesAll;
        std::vector<unsigned int> recordedMessagesLocal;
        std::vector<std::pair<std::string, unsigned int>> recordedMessagesShared;

        void updateOpinion(const faabric::Message &msg);

        void incrementInFlightCount(const faabric::Message &msg);

        void decrementInFlightCount(const faabric::Message &msg);

        void incrementWarmFaasletCount(const faabric::Message &msg);

        void decrementWarmFaasletCount(const faabric::Message &msg);

        int getFunctionMaxInFlightRatio(const faabric::Message &msg);

        ExecGraphNode getFunctionExecGraphNode(unsigned int msgId);
    };

    Scheduler &getScheduler();
}