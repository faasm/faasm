#include "Scheduler.h"

#include <util/logging.h>
#include <util/random.h>

using namespace util;

namespace scheduler {
    const std::string WARM_SET_PREFIX = "w_";

    Scheduler::Scheduler() :
            hostname(util::getHostName()),
            conf(util::getSystemConfig()),
            globalQueue(GlobalMessageQueue(INCOMING_QUEUE)),
            redis(redis::Redis::getQueue()) {

        bindQueue = new InMemoryMessageQueue();

        // Add to global set of workers
        redis.sadd(GLOBAL_WORKER_SET, hostname);
    }

    std::string Scheduler::getHostSharingQueueName() {
        return "share_" + hostname;
    }

    GlobalMessageQueue &Scheduler::getGlobalQueue() {
        return globalQueue;
    }

    void Scheduler::clear() {
        bindQueue->reset();

        for (const auto &iter: queueMap) {
            delete iter.second;
        }

        queueMap.clear();
        threadCountMap.clear();

        // Make sure this host is in the global set
        redis.sadd(GLOBAL_WORKER_SET, hostname);
    }

    Scheduler::~Scheduler() {
        delete bindQueue;

        for (const auto &iter: queueMap) {
            delete iter.second;
        }

        queueMap.clear();
    }

    void Scheduler::enqueueMessage(const message::Message &msg) {
        std::string funcStr = util::funcToString(msg);

        if (msg.type() == message::Message_MessageType_BIND) {
            bindQueue->enqueue(msg);
        } else {
            InMemoryMessageQueue *q = this->getFunctionQueue(msg);
            q->enqueue(msg);
        }
    }

    // Not thread-safe
    long Scheduler::getFunctionThreadCount(const message::Message &msg) {
        std::string funcStr = util::funcToString(msg);
        return threadCountMap[funcStr];
    }

    // Not thread-safe
    double Scheduler::getFunctionQueueRatio(const message::Message &msg) {
        std::string funcStr = util::funcToString(msg);

        long threadCount = threadCountMap[funcStr];
        long queueLength = this->getFunctionQueue(msg)->size();

        if (threadCount == 0) {
            return 100000.0;
        }

        return ((double) queueLength) / threadCount;
    }

    // Not thread-safe
    long Scheduler::getFunctionQueueLength(const message::Message &msg) {
        std::string funcStr = util::funcToString(msg);
        InMemoryMessageQueue *q = this->getFunctionQueue(msg);

        return q->size();
    }

    InMemoryMessageQueue *Scheduler::getFunctionQueue(const message::Message &msg) {
        std::string funcStr = util::funcToString(msg);
        if (queueMap.count(funcStr) == 0) {
            util::FullLock lock(mx);

            if (queueMap.count(funcStr) == 0) {
                auto mq = new InMemoryMessageQueue();

                queueMap.emplace(InMemoryMessageQueuePair(funcStr, mq));
            }
        }

        return queueMap[funcStr];
    }

    InMemoryMessageQueue *Scheduler::listenToQueue(const message::Message &msg) {
        // Note: don't need to increment thread count here as that's done when we
        // dispatch the bind message
        std::string funcStr = util::funcToString(msg);

        InMemoryMessageQueue *q = this->getFunctionQueue(msg);

        return q;
    }

    void Scheduler::stopListeningToQueue(const message::Message &msg) {
        std::string funcStr = util::funcToString(msg);

        {
            util::FullLock lock(mx);
            threadCountMap[funcStr]--;

            if (threadCountMap[funcStr] == 0) {
                std::string workerSet = this->getFunctionWarmSetName(msg);
                redis.srem(workerSet, hostname);
            }
        }
    }

    std::string Scheduler::getFunctionWarmSetName(const message::Message &msg) {
        std::string funcStr = util::funcToString(msg);
        return WARM_SET_PREFIX + funcStr;
    }

    InMemoryMessageQueue *Scheduler::getBindQueue() {
        return bindQueue;
    }

    Scheduler &getScheduler() {
        static Scheduler scheduler;
        return scheduler;
    }

    void Scheduler::callFunction(message::Message &msg) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        std::string bestHost = this->getBestHostForFunction(msg);
        if (bestHost == hostname) {
            logger->info("SCH - Executing {} locally", util::funcToString(msg));

            // Enqueue the message locally
            this->enqueueMessage(msg);

            // Add more threads if necessary
            this->addWarmThreads(msg);
        } else {
            // Share with other host
            logger->debug("Sharing {} call with {}", util::funcToString(msg), bestHost);

            // TODO - cache these queues and reuse
            GlobalMessageQueue globalQueue(bestHost);
            globalQueue.enqueueMessage(msg);
        }
    }

    void Scheduler::addWarmThreads(const message::Message &msg) {
        const std::shared_ptr<spdlog::logger> logger = util::getLogger();
        // Get the max queue ratio for this function
        int maxQueueRatio = conf.maxQueueRatio;

        // TODO make this configurable
        if (msg.function() == "sgd_step") {
            // No queueing
            maxQueueRatio = 2;
        }

        double queueRatio = this->getFunctionQueueRatio(msg);
        double nThreads = this->getFunctionThreadCount(msg);

        logger->info("SCH - {} Queue ratio = {} threads = {}", util::funcToString(msg), queueRatio, nThreads);

        // If we're over the queue ratio and have capacity, need to scale up
        if (queueRatio > maxQueueRatio && nThreads < conf.maxWorkersPerFunction) {
            FullLock lock(mx);

            std::string funcStr = util::funcToString(msg);

            queueRatio = this->getFunctionQueueRatio(msg);
            nThreads = this->getFunctionThreadCount(msg);

            // Double check condition
            if (queueRatio > maxQueueRatio && nThreads < conf.maxWorkersPerFunction) {
                logger->info("SCH - Scaling up {} to {} threads", util::funcToString(msg), nThreads + 1);

                // If this is the first thread on this host, add it to the warm set for this function
                if (nThreads == 0) {
                    std::string workerSet = this->getFunctionWarmSetName(msg);
                    redis.sadd(workerSet, hostname);
                }

                // Increment thread count here
                threadCountMap[funcStr]++;

                message::Message bindMsg;
                bindMsg.set_type(message::Message_MessageType_BIND);
                bindMsg.set_user(msg.user());
                bindMsg.set_function(msg.function());

                this->enqueueMessage(bindMsg);
            }
        }
    }

    std::string Scheduler::getBestHostForFunction(const message::Message &msg) {
        const std::shared_ptr<spdlog::logger> logger = util::getLogger();

        // If we're ignoring the scheduling, just put it on this host regardless
        bool ignoreScheduler = conf.noScheduler == 1;
        if(ignoreScheduler) {
            logger->debug("Ignoring scheduler and queueing {} locally", util::funcToString(msg));
            return hostname;
        }

        bool excludeThisHost = false;

        {
            SharedLock lock(mx);
                          
            // If we have some warm threads below the max, we can handle locally
            long threadCount = this->getFunctionThreadCount(msg);
            if (threadCount > 0 && threadCount < conf.maxWorkersPerFunction) {
                return hostname;
            } else if (threadCount == conf.maxWorkersPerFunction) {
                // If we're at the max workers, we want to saturate so that all are full,
                // i.e. we want to get up to the maximum queue ratio
                double queueRatio = this->getFunctionQueueRatio(msg);

                if(queueRatio >= conf.maxQueueRatio) {
                    // Only exclude when we've saturated the last worker
                    excludeThisHost = true;
                }
            }
        }

        std::string warmSet = this->getFunctionWarmSetName(msg);

        // Get options excluding this host
        std::unordered_set<std::string> warmOptions = redis.smembers(warmSet);
        std::unordered_set<std::string> allOptions = redis.smembers(GLOBAL_WORKER_SET);
        warmOptions.erase(hostname);
        allOptions.erase(hostname);

        std::string hostChoice;

        if (!warmOptions.empty()) {
            // If we have warm options, pick one of those
            return *warmOptions.begin();
        } else if (!excludeThisHost) {
            // If no warm options, we should accommodate on this host unless it's over the limit
            return hostname;
        } else if (!allOptions.empty()) {
            // Pick a random option from all hosts
            return *allOptions.begin();
        } else {
            // TODO: scale out here
            logger->error("No worker host available for scheduling {}", util::funcToString(msg));
            throw std::runtime_error("No worker host available for scheduling");
        }
    }
}