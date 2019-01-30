#pragma once

#include <shared_mutex>
#include <redis/redis.h>
#include <util/util.h>

#define GLOBAL_WORKER_SET "available_workers"

namespace scheduler {
    const std::string INCOMING_QUEUE = "incoming";

    typedef std::queue<message::Message> InMemoryMessageQueue;
    typedef std::pair<std::string, InMemoryMessageQueue*> InMemoryMessageQueuePair;

    class LocalQueueMap {
    public:
        LocalQueueMap();
        ~LocalQueueMap();

        LocalQueueMap & getInstance();

        long getFunctionThreadCount(const message::Message &msg);
        long getFunctionQueueLength(const message::Message &msg);

        InMemoryMessageQueue* listenToQueue(const message::Message &msg);
        InMemoryMessageQueue* getFunctionQueue(const message::Message &msg);
        void stopListeningToQueue(const message::Message &msg);
        InMemoryMessageQueue* getBindQueue();

    private:
        InMemoryMessageQueue *bindQueue;
        std::shared_mutex mx;
        std::map<std::string, InMemoryMessageQueue*> queueMap;
        std::map<std::string, long> threadCountMap;
    };

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

        const int scheduleWaitMillis = 100;

        const int scheduleRecursionLimit = 10;

        long getFunctionCount(const message::Message &msg);

        long getFunctionQueueLength(const message::Message &msg);

        long getLocalThreadCount(const message::Message &msg);

        void workerBound(const message::Message &msg);

        void workerUnbound(const message::Message &msg);

        void addCurrentHostToWorkerPool();

        void placeOnGlobalQueue(message::Message &msg);

        std::string callFunction(message::Message &msg);

        std::string getFunctionQueueName(const message::Message &msg);

        std::string getFunctionWorkerSetName(const message::Message &msg);

        std::string getHostPrewarmQueue(const std::string &hostName);

        std::string getHostPrewarmQueue();

        std::string getPrewarmQueueForFunction(const message::Message &msg, bool affinity);

        void reset();

    private:
        std::map<std::string, int> funcCounts;
        std::mutex funcCountMutex;
        std::string hostname;

        void updateWorkerAllocs(const message::Message &msg, int recursionCount = 0);

        std::string getFunctionCounterName(const message::Message &msg);

        MessageQueue messageQueue;
        util::SystemConfig &conf;
    };

    Scheduler &getScheduler();
}