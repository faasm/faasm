#pragma once

#include <redis/redis.h>
#include <util/util.h>

#define GLOBAL_WORKER_SET "available_workers"

namespace scheduler {
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

        redis::Redis &redis;
        util::SystemConfig &conf;
    };

    Scheduler &getScheduler();
}