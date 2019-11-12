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

    void Scheduler::addNodeToGlobalSet(const std::string &node) {
        redis::Redis &redis = redis::Redis::getQueue();
        redis.sadd(GLOBAL_NODE_SET, node);
    }

    void Scheduler::addNodeToGlobalSet() {
        redis::Redis &redis = redis::Redis::getQueue();
        redis.sadd(GLOBAL_NODE_SET, nodeId);
    }

    void Scheduler::removeNodeFromGlobalSet() {
        redis::Redis &redis = redis::Redis::getQueue();
        redis.srem(GLOBAL_NODE_SET, nodeId);
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

    void Scheduler::clear() {
        // Remove this node from all the global warm sets
        for (const auto &iter: queueMap) {
            this->removeNodeFromWarmSet(iter.first);
        }

        // Clear anything in the bind queue
        bindQueue->reset();
        queueMap.clear();
        threadCountMap.clear();
        inFlightCountMap.clear();
        opinionMap.clear();

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

    long Scheduler::getFunctionThreadCount(const message::Message &msg) {
        std::string funcStr = util::funcToString(msg, false);
        return threadCountMap[funcStr];
    }

    double Scheduler::getFunctionInFlightRatio(const message::Message &msg) {
        std::string funcStr = util::funcToString(msg, false);

        long threadCount = threadCountMap[funcStr];
        long inFlightCount = getFunctionInFlightCount(msg);

        if (threadCount == 0) {
            return 0;
        }

        return ((double) inFlightCount) / threadCount;
    }

    long Scheduler::getFunctionInFlightCount(const message::Message &msg) {
        return inFlightCountMap[util::funcToString(msg, false)];
    }

    std::shared_ptr<InMemoryMessageQueue> Scheduler::getFunctionQueue(const message::Message &msg) {
        std::string funcStr = util::funcToString(msg, false);

        // This will be called from within something holding the lock
        if (queueMap.count(funcStr) == 0) {
            if (queueMap.count(funcStr) == 0) {
                auto mq = new InMemoryMessageQueue();
                queueMap.emplace(InMemoryMessageQueuePair(funcStr, mq));
            }
        }

        return queueMap[funcStr];
    }

    void Scheduler::notifyCallFinished(const message::Message &msg) {
        util::FullLock lock(mx);

        // Decrement the in-flight count
        const std::string funcStr = util::funcToString(msg, false);
        inFlightCountMap[funcStr] = std::max(inFlightCountMap[funcStr] - 1 , 0L);

        updateOpinion(msg);
    }

    void Scheduler::notifyThreadFinished(const message::Message &msg) {
        std::string funcStr = util::funcToString(msg, false);

        {
            util::FullLock lock(mx);
            threadCountMap[funcStr] = std::max(threadCountMap[funcStr] - 1 , 0L);

            updateOpinion(msg);
        }
    }

    void Scheduler::notifyAwaiting(const message::Message &msg) {
        std::string funcStr = util::funcToString(msg, false);

        // When a thread is awaiting a call, we can reduce the thread count
        // as it's doing a non-blocking wait, then we can potentially add more
        {
            util::FullLock lock(mx);
            threadCountMap[funcStr] = std::max(threadCountMap[funcStr] - 1 , 0L);

            addWarmThreads(msg);

            updateOpinion(msg);
        }
    }

    void Scheduler::notifyFinishedAwaiting(const message::Message &msg) {
        std::string funcStr = util::funcToString(msg, false);

        // When a thread returns from awaiting, we can increase the thread count again
        {
            util::FullLock lock(mx);
            threadCountMap[funcStr]++;

            updateOpinion(msg);
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

    Scheduler &getScheduler() {
        // This is *global* and must be shared across threads
        static Scheduler scheduler;
        return scheduler;
    }

    void Scheduler::callFunction(message::Message &msg, bool forceLocal) {
        util::FullLock lock(mx);
        PROF_START(scheduleCall)

        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        // Get the best node
        std::string bestNode;
        if(forceLocal) {
            bestNode = nodeId;
        } else {
            bestNode = this->getBestNodeForFunction(msg);
        }

        // Mark if this is the first scheduling decision made on this message
        if (msg.schedulednode().empty()) {
            msg.set_schedulednode(bestNode);
        }

        const std::string funcStrWithId = util::funcToString(msg, true);
        const std::string funcStrNoId = util::funcToString(msg, false);

        if (bestNode == nodeId) {
            // Run locally if we're the best choice
            logger->debug("Executing {} locally", funcStrWithId);
            this->enqueueMessage(msg);

            // Increment the in-flight count
            inFlightCountMap[funcStrNoId]++;

            // Add more threads if necessary
            this->addWarmThreads(msg);

            // Update our opinion
            updateOpinion(msg);
        } else {
            // Increment the number of hops
            msg.set_hops(msg.hops() + 1);

            // Share with other node
            logger->debug("Node {} sharing {} with {} ({} hops)",
                          nodeId, funcStrWithId,
                          bestNode, msg.hops());

            sharingBus.shareMessageWithNode(bestNode, msg);
        }

        PROF_END(scheduleCall)
    }

    void Scheduler::addWarmThreads(const message::Message &msg) {
        const std::shared_ptr<spdlog::logger> logger = util::getLogger();

        int maxInFlightRatio = conf.maxInFlightRatio;
        double inFlightRatio = this->getFunctionInFlightRatio(msg);
        long nThreads = this->getFunctionThreadCount(msg);

        const std::string funcStr = util::funcToString(msg, false);
        logger->debug("{} IF ratio = {} (max {}) threads = {}", funcStr, inFlightRatio, maxInFlightRatio, nThreads);

        // If we have no threads OR if we've got threads and are at or over the in-flight ratio
        // and have capacity, then we need to scale up
        bool needToScale = false;
        needToScale |= nThreads == 0;
        needToScale |= inFlightRatio >= maxInFlightRatio && nThreads < conf.maxWorkersPerFunction;

        if (needToScale) {
            logger->debug("Scaling up {} to {} threads", funcStr, nThreads + 1);

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

    std::string opinionStr(const SchedulerOpinion &o) {
        switch (o) {
            case (MAYBE): {
                return "MAYBE";
            }
            case (YES): {
                return "YES";
            }
            case (NO): {
                return "NO";
            }
            default:
                return "UNKNOWN";
        }
    }

    void Scheduler::updateOpinion(const message::Message &msg) {
        // Lock-free, should be called when lock held
        const std::string funcStr = util::funcToString(msg, false);

        // Check the thread capacity
        long threadCount = this->getFunctionThreadCount(msg);
        bool hasWarmThreads = threadCount > 0;
        bool atMaxThreads = threadCount >= conf.maxWorkersPerFunction;

        // Check the in-flight ratio
        double inFlightRatio = this->getFunctionInFlightRatio(msg);
        int maxInFlightRatio = conf.maxInFlightRatio;
        bool isInFlightRatioBreached = inFlightRatio >= maxInFlightRatio;

        SchedulerOpinion newOpinion;

        if (isInFlightRatioBreached) {
            if (atMaxThreads) {
                // If in-flight ratio is breached and we're at the max, we're a definite NO
                newOpinion = NO;
            } else {
                // If in-flight ratio is breached but we can add more threads, we're a YES
                newOpinion = YES;
            }
        } else if (hasWarmThreads) {
            // If we've not breached the in-flight ratio and we have some warm threads, we're a YES
            newOpinion = YES;
        } else {
            // If we have no threads we're a maybe
            newOpinion = MAYBE;
        }

        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        SchedulerOpinion currentOpinion = opinionMap[funcStr];
        if (newOpinion != currentOpinion) {
            std::string newOpinionStr = opinionStr(newOpinion);
            std::string currentOpinionStr = opinionStr(currentOpinion);

            logger->debug(
                    "{} updating {} from {} to {} (threads={} ({}), IF ratio={} ({}))",
                    nodeId,
                    funcStr,
                    currentOpinionStr,
                    newOpinionStr,
                    threadCount,
                    conf.maxWorkersPerFunction,
                    inFlightRatio,
                    maxInFlightRatio
            );

            if (newOpinion == NO) {
                // Moving to no means we want to switch off from all decisions
                removeNodeFromWarmSet(funcStr);
                removeNodeFromGlobalSet();
            } else if (newOpinion == MAYBE && currentOpinion == NO) {
                // Rejoin the global set if we're now a maybe when previously a no
                addNodeToGlobalSet();
            } else if (newOpinion == MAYBE && currentOpinion == YES) {
                // Stay in the global set, but not in the warm if we've gone back to maybe from yes
                removeNodeFromWarmSet(funcStr);
            } else if (newOpinion == YES && currentOpinion == NO) {
                // Rejoin everything if we're into a yes state from a no
                addNodeToWarmSet(funcStr);
                addNodeToGlobalSet();
            } else if (newOpinion == YES && currentOpinion == MAYBE) {
                // Join the warm set if we've become yes from maybe
                addNodeToWarmSet(funcStr);
            } else {
                throw std::logic_error("Should not be able to reach this point");
            }

            // Finally update the map
            opinionMap[funcStr] = newOpinion;
        }
    }

    SchedulerOpinion Scheduler::getOpinion(const message::Message &msg) {
        const std::string funcStr = funcToString(msg, false);
        return opinionMap[funcStr];
    }

    std::string Scheduler::getBestNodeForFunction(const message::Message &msg) {
        const std::shared_ptr<spdlog::logger> logger = util::getLogger();

        // If we're ignoring the scheduling, just put it on this node regardless
        if (conf.noScheduler == 1) {
            logger->debug("Ignoring scheduler and queueing {} locally", util::funcToString(msg, true));
            return nodeId;
        }

        // Accept if we have capacity
        const std::string funcStrNoId = util::funcToString(msg, false);
        SchedulerOpinion thisOpinion = opinionMap[funcStrNoId];
        if (thisOpinion == YES) {
            return nodeId;
        }

        // Get options from the warm set
        std::string warmSet = this->getFunctionWarmSetName(msg);
        redis::Redis &redis = redis::Redis::getQueue();
        std::unordered_set<std::string> warmOptions = redis.smembers(warmSet);

        // Remove this node from the warm options
        warmOptions.erase(nodeId);

        // If we have warm options, pick a random one
        if (!warmOptions.empty()) {
            return util::randomStringFromSet(warmOptions);
        }

        // If there are no other warm options and we're a maybe, accept on this node
        if (thisOpinion == MAYBE) {
            return nodeId;
        }

        // Now there's no warm options and we're rejecting, so check all options
        std::unordered_set<std::string> allOptions = redis.smembers(GLOBAL_NODE_SET);
        allOptions.erase(nodeId);

        if (!allOptions.empty()) {
            // Pick a random option from all nodes
            return util::randomStringFromSet(allOptions);
        } else {
            // Give up and try to execute locally
            double inFlightRatio = getFunctionInFlightRatio(msg);
            const std::string oStr = opinionStr(thisOpinion);
            logger->warn("{} overloaded for {}. IF ratio {}, opinion {}", nodeId, funcStrNoId, inFlightRatio,
                         oStr);
            return nodeId;
        }
    }
}