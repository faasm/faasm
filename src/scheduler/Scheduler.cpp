#include "Scheduler.h"

#include <util/logging.h>
#include <util/random.h>
#include <util/timing.h>
#include <scheduler/SharingMessageBus.h>

using namespace util;

namespace scheduler {
    const std::string WARM_SET_PREFIX = "w_";

    Scheduler::Scheduler() :
            nodeId(util::getNodeId()),
            conf(util::getSystemConfig()),
            sharingBus(SharingMessageBus::getInstance()) {

        bindQueue = std::make_shared<InMemoryMessageQueue>();
    }

    void Scheduler::addNodeToGlobalSet() {
        // Add to global set of workers
        redis::Redis &redis = redis::Redis::getQueue();
        redis.sadd(GLOBAL_NODE_SET, nodeId);
    }

    void Scheduler::removeNodeFromGlobalSet() {
        redis::Redis &redis = redis::Redis::getQueue();
        redis.srem(GLOBAL_NODE_SET, nodeId);
    }

    long Scheduler::getGlobalSetSize() {
        redis::Redis &redis = redis::Redis::getQueue();
        return redis.scard(GLOBAL_NODE_SET);
    }

    void Scheduler::addNodeToWarmSet(const std::string &funcStr) {
        const std::string &warmSetName = getFunctionWarmSetNameFromStr(funcStr);
        redis::Redis &redis = redis::Redis::getQueue();
        redis.sadd(warmSetName, nodeId);
    }

    void Scheduler::removeNodeFromWarmSet(const std::string &funcStr) {
        const std::string &warmSetName = getFunctionWarmSetNameFromStr(funcStr);
        redis::Redis &redis = redis::Redis::getQueue();
        redis.srem(warmSetName, nodeId);
    }

    int Scheduler::getExecutingCount() {
        util::SharedLock lock(execCountMutex);
        return execCount;
    }

    void Scheduler::incrementExecutingCount() {
        util::FullLock lock(execCountMutex);
        execCount++;
    }

    void Scheduler::decrementExecutingCount() {
        util::FullLock lock(execCountMutex);

        if (execCount > 0) {
            execCount--;
        }
    }

    void Scheduler::clear() {
        // Remove each node from the relevant warm sets
        for (const auto &iter: queueMap) {
            this->removeNodeFromWarmSet(iter.first);
        }

        bindQueue->reset();
        queueMap.clear();
        threadCountMap.clear();

        this->removeNodeFromGlobalSet();
    }

    void Scheduler::enqueueMessage(const message::Message &msg) {
        if (msg.type() == message::Message_MessageType_BIND) {
            bindQueue->enqueue(msg);
        } else {
            auto q = this->getFunctionQueue(msg);
            q->enqueue(msg);
        }
    }

    // Not thread-safe
    long Scheduler::getFunctionThreadCount(const message::Message &msg) {
        std::string funcStr = util::funcToString(msg, false);
        return threadCountMap[funcStr];
    }

    // Not thread-safe
    double Scheduler::getFunctionQueueRatio(const message::Message &msg) {
        std::string funcStr = util::funcToString(msg, false);

        long threadCount = threadCountMap[funcStr];
        long queueLength = this->getFunctionQueue(msg)->size();

        if (threadCount == 0) {
            return 100000.0;
        }

        return ((double) queueLength) / threadCount;
    }

    // Not thread-safe
    long Scheduler::getFunctionQueueLength(const message::Message &msg) {
        auto q = this->getFunctionQueue(msg);
        return q->size();
    }

    std::shared_ptr<InMemoryMessageQueue> Scheduler::getFunctionQueue(const message::Message &msg) {
        std::string funcStr = util::funcToString(msg, false);
        if (queueMap.count(funcStr) == 0) {
            util::FullLock lock(mx);

            if (queueMap.count(funcStr) == 0) {
                auto mq = new InMemoryMessageQueue();

                queueMap.emplace(InMemoryMessageQueuePair(funcStr, mq));
            }
        }

        return queueMap[funcStr];
    }

    std::shared_ptr<InMemoryMessageQueue> Scheduler::listenToQueue(const message::Message &msg) {
        // Note: don't need to increment thread count here as that's done when we
        // dispatch the bind message
        auto q = this->getFunctionQueue(msg);

        return q;
    }

    void Scheduler::stopListeningToQueue(const message::Message &msg) {
        std::string funcStr = util::funcToString(msg, false);

        {
            util::FullLock lock(mx);
            threadCountMap[funcStr]--;

            // If this is the last thread listening to this queue, remove the node entirely
            if (threadCountMap[funcStr] == 0) {
                this->removeNodeFromWarmSet(funcStr);
            }
        }
    }

    std::string Scheduler::getFunctionWarmSetName(const message::Message &msg) {
        std::string funcStr = util::funcToString(msg, false);
        return this->getFunctionWarmSetNameFromStr(funcStr);
    }

    std::string Scheduler::getFunctionWarmSetNameFromStr(const std::string &funcStr) {
        return WARM_SET_PREFIX + funcStr;
    }

    std::shared_ptr<InMemoryMessageQueue> Scheduler::getBindQueue() {
        return bindQueue;
    }

    int getMaxQueueRatio(const message::Message &msg) {
        // When this is a normal call the max queue ratio will be the standard config value.
        // When executing a chained call we want to execute immediately if there's a free thread,
        // or scale out immediately.

        bool isChained = msg.idx() > 0;
        if (isChained) {
            return 1;
        } else {
            SystemConfig &conf = util::getSystemConfig();
            return conf.maxQueueRatio;
        }
    }

    Scheduler &getScheduler() {
        // This is *global* and must be shared across threads
        static Scheduler scheduler;
        return scheduler;
    }

    void Scheduler::callFunction(message::Message &msg) {
        PROF_START(scheduleCall)
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        std::string thisNodeId = util::getNodeId();
        std::string bestNode = this->getBestNodeForFunction(msg);
        const std::string funcStr = util::funcToString(msg, true);

        // If this is the first time the message has been scheduled, mark this
        // as the scheduled node
        if (msg.schedulednode().empty()) {
            msg.set_schedulednode(bestNode);
        }

        if (bestNode == nodeId) {
            logger->debug("Executing {} locally", funcStr);

            // Enqueue the message locally
            this->enqueueMessage(msg);

            // Add more threads if necessary
            this->addWarmThreads(msg);
        } else {
            // Increment the number of hops
            msg.set_hops(msg.hops() + 1);

            // Share with other node
            logger->debug("Node {} sharing {} with {} ({} hops)", thisNodeId, funcStr, bestNode, msg.hops());
            sharingBus.shareMessageWithNode(bestNode, msg);
        }

        PROF_END(scheduleCall)
    }

    void Scheduler::addWarmThreads(const message::Message &msg) {
        const std::shared_ptr<spdlog::logger> logger = util::getLogger();

        int maxQueueRatio = getMaxQueueRatio(msg);

        double queueRatio = this->getFunctionQueueRatio(msg);
        long nThreads = this->getFunctionThreadCount(msg);

        const std::string funcStr = util::funcToString(msg, false);
        logger->debug("{} queue ratio = {} (max {}) threads = {}", funcStr, queueRatio, maxQueueRatio, nThreads);

        // If we're over or at the queue ratio and have capacity, need to scale up
        if (queueRatio >= maxQueueRatio && nThreads < conf.maxWorkersPerFunction) {
            FullLock lock(mx);

            queueRatio = this->getFunctionQueueRatio(msg);
            nThreads = this->getFunctionThreadCount(msg);

            // Double check condition
            if (queueRatio >= maxQueueRatio && nThreads < conf.maxWorkersPerFunction) {
                logger->debug("Scaling up {} to {} threads", funcStr, nThreads + 1);

                // If this is the first thread on this node, add it to the warm set for this function
                if (nThreads == 0) {
                    this->addNodeToWarmSet(funcStr);
                }

                // Increment thread count here
                threadCountMap[funcStr]++;

                // Send bind message (i.e. request a thread)
                message::Message bindMsg = util::messageFactory(msg.user(), msg.function());
                bindMsg.set_type(message::Message_MessageType_BIND);
                bindMsg.set_ispython(msg.ispython());
                bindMsg.set_istypescript(msg.istypescript());

                this->enqueueMessage(bindMsg);
            }
        }
    }

    std::string Scheduler::getBestNodeForFunction(const message::Message &msg) {
        const std::shared_ptr<spdlog::logger> logger = util::getLogger();

        // If we're ignoring the scheduling, just put it on this node regardless
        if (conf.noScheduler == 1) {
            logger->debug("Ignoring scheduler and queueing {} locally", util::funcToString(msg, true));
            return nodeId;
        }

        bool excludeThisNode = false;

        {
            SharedLock lock(mx);

            // Get the thread count for this function on this node
            long threadCount = this->getFunctionThreadCount(msg);

            // If we have some warm threads below the max, we can handle locally
            if (threadCount > 0 && threadCount < conf.maxWorkersPerFunction) {
                return nodeId;
            } else if (threadCount >= conf.maxWorkersPerFunction) {
                // If we're at/ above the max workers, we want to saturate so that all
                // are full, i.e. we want to get up to the maximum queue ratio
                double queueRatio = this->getFunctionQueueRatio(msg);
                int maxQueueRatio = getMaxQueueRatio(msg);

                if (queueRatio >= maxQueueRatio) {
                    // Only exclude when we've saturated the last worker
                    excludeThisNode = true;
                }
            }
        }

        std::string warmSet = this->getFunctionWarmSetName(msg);

        // Get options excluding this node
        redis::Redis &redis = redis::Redis::getQueue();
        std::unordered_set<std::string> warmOptions = redis.smembers(warmSet);
        std::unordered_set<std::string> allOptions = redis.smembers(GLOBAL_NODE_SET);
        int nCurrentWorkers = (int) allOptions.size();

        // Remove this node from sets
        warmOptions.erase(nodeId);
        allOptions.erase(nodeId);

        std::string nodeChoice;

        if (!warmOptions.empty()) {
            // If we have warm options, pick one of those
            return *warmOptions.begin();
        } else if (!excludeThisNode) {
            // If no warm options, we should accommodate on this node unless it's over the limit
            return nodeId;
        } else if (!allOptions.empty()) {
            // Pick a random option from all nodes
            return *allOptions.begin();
        } else {
            // Request scale out, then give up and try to execute locally
            this->scaleOut(nCurrentWorkers + 1);

            return nodeId;
        }
    }

    void Scheduler::scaleOut(int targetCount) {
        long nActiveWorkers = this->getGlobalSetSize();
        const std::shared_ptr<spdlog::logger> logger = util::getLogger();

        if ((targetCount > nActiveWorkers) && (nActiveWorkers < conf.maxNodes)) {
            logger->info("Requesting scale-out (from {} -> {} nodes)", nActiveWorkers, targetCount);

            // Attempt to spawn a new worker
            GlobalMessageBus &globalBus = getGlobalMessageBus();
            globalBus.requestNewWorkerNode();
        }
    }
}