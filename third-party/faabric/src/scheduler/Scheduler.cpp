#include "Scheduler.h"

#include <faabric/util/logging.h>
#include <faabric/util/random.h>
#include <faabric/util/timing.h>
#include <faabric/scheduler/FunctionCallClient.h>
#include <faabric/redis/Redis.h>

#define CHAINED_SET_PREFIX "chained_"


using namespace faabric::util;

namespace faabric::scheduler {
    const std::string WARM_SET_PREFIX = "w_";

    std::string getChainedKey(unsigned int msgId) {
        return std::string(CHAINED_SET_PREFIX) + std::to_string(msgId);
    }
    
    Scheduler::Scheduler() :
            thisHost(faabric::utilgetSystemConfig().endpointHost),
            conf(faabric::utilgetSystemConfig()),
            isTestMode(false) {

        bindQueue = std::make_shared<InMemoryMessageQueue>();
    }

    void Scheduler::addHostToGlobalSet(const std::string &host) {
        redis::Redis &redis = redis::Redis::getQueue();
        redis.sadd(AVAILABLE_HOST_SET, host);
    }

    void Scheduler::addHostToGlobalSet() {
        redis::Redis &redis = redis::Redis::getQueue();
        redis.sadd(AVAILABLE_HOST_SET, thisHost);
    }

    void Scheduler::removeHostFromGlobalSet() {
        redis::Redis &redis = redis::Redis::getQueue();
        redis.srem(AVAILABLE_HOST_SET, thisHost);
    }

    void Scheduler::addHostToWarmSet(const std::string &funcStr) {
        const std::string &warmSetName = getFunctionWarmSetNameFromStr(funcStr);
        redis::Redis &redis = redis::Redis::getQueue();
        redis.sadd(warmSetName, thisHost);
    }

    void Scheduler::removeHostFromWarmSet(const std::string &funcStr) {
        const std::string &warmSetName = getFunctionWarmSetNameFromStr(funcStr);
        redis::Redis &redis = redis::Redis::getQueue();
        redis.srem(warmSetName, thisHost);
    }

    void Scheduler::clear() {
        // Remove this host from all the global warm sets
        for (const auto &iter: queueMap) {
            this->removeHostFromWarmSet(iter.first);
        }

        // Reset host
        thisHost = faabric::utilgetSystemConfig().endpointHost;

        // Clear all queues
        for (const auto &p: queueMap) {
            p.second->reset();
        }
        queueMap.clear();
        bindQueue->reset();

        // Reset all state
        faasletCountMap.clear();
        inFlightCountMap.clear();
        opinionMap.clear();
        _hasHostCapacity = true;

        // Records
        setTestMode(false);
        recordedMessagesAll.clear();
        recordedMessagesLocal.clear();
        recordedMessagesShared.clear();

        this->removeHostFromGlobalSet();
    }

    void Scheduler::enqueueMessage(const faabric::Message &msg) {
        if (msg.type() == faabric::Message_MessageType_BIND) {
            bindQueue->enqueue(msg);
        } else {
            auto q = this->getFunctionQueue(msg);
            q->enqueue(msg);
        }
    }

    long Scheduler::getFunctionWarmFaasletCount(const faabric::Message &msg) {
        std::string funcStr = faabric::utilfuncToString(msg, false);
        return faasletCountMap[funcStr];
    }

    double Scheduler::getFunctionInFlightRatio(const faabric::Message &msg) {
        std::string funcStr = faabric::utilfuncToString(msg, false);

        long faasletCount = faasletCountMap[funcStr];
        long inFlightCount = getFunctionInFlightCount(msg);

        if (faasletCount == 0) {
            return 0;
        }

        return ((double) inFlightCount) / faasletCount;
    }

    int Scheduler::getFunctionMaxInFlightRatio(const faabric::Message &msg) {
        int maxInFlightRatio = conf.maxInFlightRatio;
        if (msg.ismpi()) {
            faabric::utilgetLogger()->debug("Overriding max in-flight ratio for MPI function ({} -> {})", maxInFlightRatio, 1);
            maxInFlightRatio = 1;
        }
        return maxInFlightRatio;
    }

    long Scheduler::getFunctionInFlightCount(const faabric::Message &msg) {
        const std::string funcStr = faabric::utilfuncToString(msg, false);
        return inFlightCountMap[funcStr];
    }

    std::shared_ptr<InMemoryMessageQueue> Scheduler::getFunctionQueue(const faabric::Message &msg) {
        std::string funcStr = faabric::utilfuncToString(msg, false);

        // This will be called from within something holding the lock
        if (queueMap.count(funcStr) == 0) {
            if (queueMap.count(funcStr) == 0) {
                auto mq = new InMemoryMessageQueue();
                queueMap.emplace(InMemoryMessageQueuePair(funcStr, mq));
            }
        }

        return queueMap[funcStr];
    }

    void Scheduler::notifyCallFinished(const faabric::Message &msg) {
        faabric::utilFullLock lock(mx);
        decrementInFlightCount(msg);
    }

    void Scheduler::notifyFaasletFinished(const faabric::Message &msg) {
        faabric::utilFullLock lock(mx);
        decrementWarmFaasletCount(msg);
    }

    std::string Scheduler::getFunctionWarmSetName(const faabric::Message &msg) {
        std::string funcStr = faabric::utilfuncToString(msg, false);
        return this->getFunctionWarmSetNameFromStr(funcStr);
    }

    std::string Scheduler::getFunctionWarmSetNameFromStr(const std::string &funcStr) {
        return WARM_SET_PREFIX + funcStr;
    }

    std::shared_ptr<InMemoryMessageQueue> Scheduler::getBindQueue() {
        return bindQueue;
    }

    Scheduler &getScheduler() {
        // This is *global* and must be shared across faaslets
        static Scheduler scheduler;
        return scheduler;
    }

    void Scheduler::callFunction(faabric::Message &msg, bool forceLocal) {
        faabric::utilFullLock lock(mx);
        PROF_START(scheduleCall)

        const std::shared_ptr <spdlog::logger> &logger = faabric::utilgetLogger();

        // Get the best host
        std::string bestHost;
        if (forceLocal) {
            bestHost = thisHost;
        } else {
            // Make sure our opinion is up to date
            updateOpinion(msg);
            bestHost = this->getBestHostForFunction(msg);
        }

        // Mark if this is the first scheduling decision made on this message
        if (msg.scheduledhost().empty()) {
            msg.set_scheduledhost(bestHost);
        }

        bool executedLocally = true;
        const std::string funcStrWithId = faabric::utilfuncToString(msg, true);
        if (bestHost == thisHost) {
            // Run locally if we're the best choice
            logger->debug("Executing {} locally", funcStrWithId);
            this->enqueueMessage(msg);

            incrementInFlightCount(msg);
        } else {
            // Increment the number of hops
            msg.set_hops(msg.hops() + 1);

            // Share with other host (or just log in test mode)
            if(isTestMode) {
                logger->debug("TEST MODE - {} not sharing {} with {} ({} hops)",
                              thisHost, funcStrWithId,
                              bestHost, msg.hops());
            } else {
                logger->debug("Host {} sharing {} with {} ({} hops)",
                              thisHost, funcStrWithId,
                              bestHost, msg.hops());

                FunctionCallClient c(bestHost);
                c.shareFunctionCall(msg);
            }

            executedLocally = false;
        }

        // Add this message to records if necessary
        if (isTestMode) {
            recordedMessagesAll.push_back(msg.id());
            if(executedLocally) {
                recordedMessagesLocal.push_back(msg.id());
            } else {
                recordedMessagesShared.emplace_back(bestHost, msg.id());
            }
        }

        PROF_END(scheduleCall)
    }

    long Scheduler::getTotalWarmFaasletCount() {
        long totalCount = 0;
        for (auto p : faasletCountMap) {
            totalCount += p.second;
        }

        return totalCount;
    }

    std::string opinionStr(const SchedulerOpinion &o) {
        switch (o) {
            case (SchedulerOpinion::MAYBE): {
                return "MAYBE";
            }
            case (SchedulerOpinion::YES): {
                return "YES";
            }
            case (SchedulerOpinion::NO): {
                return "NO";
            }
            default:
                return "UNKNOWN";
        }
    }

    void Scheduler::updateOpinion(const faabric::Message &msg) {
        // Note this function is lock-free, should be called when lock held
        const std::string funcStr = faabric::utilfuncToString(msg, false);
        SchedulerOpinion currentOpinion = opinionMap[funcStr];

        // Check per-function limits
        long faasletCount = getFunctionWarmFaasletCount(msg);
        bool hasWarmFaaslets = faasletCount > 0;
        bool hasFunctionCapacity = faasletCount < conf.maxFaasletsPerFunction;

        // Check the overall host capacity
        long totalFaasletCount = getTotalWarmFaasletCount();
        _hasHostCapacity = totalFaasletCount < conf.maxFaaslets;

        // Check the in-flight ratio
        double inFlightRatio = getFunctionInFlightRatio(msg);
        int maxInFlightRatio = getFunctionMaxInFlightRatio(msg);
        bool isInFlightRatioBreached = inFlightRatio >= maxInFlightRatio;

        SchedulerOpinion newOpinion;

        if (isInFlightRatioBreached) {
            // If in-flight ratio breached, we need more capacity.
            // If both the function and host have capacity, it's YES, otherwise NO.
            if (hasFunctionCapacity && _hasHostCapacity) {
                newOpinion = SchedulerOpinion::YES;
            } else {
                newOpinion = SchedulerOpinion::NO;
            }
        } else if (hasWarmFaaslets) {
            // If we've not breached the in-flight ratio and we have some warm faaslets, it's YES
            newOpinion = SchedulerOpinion::YES;
        } else if (!_hasHostCapacity) {
            // If we have no warm faaslets and no host capacity, it's NO
            newOpinion = SchedulerOpinion::NO;
        } else {
            // In all other scenarios it's MAYBE
            newOpinion = SchedulerOpinion::MAYBE;
        }

        const std::shared_ptr <spdlog::logger> &logger = faabric::utilgetLogger();
        if (newOpinion != currentOpinion) {
            std::string newOpinionStr = opinionStr(newOpinion);
            std::string currentOpinionStr = opinionStr(currentOpinion);

            logger->debug(
                    "{} updating {} from {} to {} (faaslets={} ({}), IF ratio={} ({}))",
                    thisHost,
                    funcStr,
                    currentOpinionStr,
                    newOpinionStr,
                    faasletCount,
                    conf.maxFaasletsPerFunction,
                    inFlightRatio,
                    maxInFlightRatio
            );

            if (newOpinion == SchedulerOpinion::NO) {
                // Moving to NO means we want to remove ourself from the set for this function
                removeHostFromWarmSet(funcStr);

                // If we've also reached this host's capacity, we want to drop out from all
                // other scheduling decisions
                if (!_hasHostCapacity) {
                    removeHostFromGlobalSet();
                }
            } else if (newOpinion == SchedulerOpinion::MAYBE && currentOpinion == SchedulerOpinion::NO) {
                // Rejoin the global set if we're now a maybe when previously a no
                addHostToGlobalSet();
            } else if (newOpinion == SchedulerOpinion::MAYBE && currentOpinion == SchedulerOpinion::YES) {
                // Stay in the global set, but not in the warm if we've gone back to maybe from yes
                removeHostFromWarmSet(funcStr);
            } else if (newOpinion == SchedulerOpinion::YES && currentOpinion == SchedulerOpinion::NO) {
                // Rejoin everything if we're into a yes state from a no
                addHostToWarmSet(funcStr);
                addHostToGlobalSet();
            } else if (newOpinion == SchedulerOpinion::YES && currentOpinion == SchedulerOpinion::MAYBE) {
                // Join the warm set if we've become yes from maybe
                addHostToWarmSet(funcStr);
            } else {
                throw std::logic_error("Should not be able to reach this point");
            }

            // Finally update the map
            opinionMap[funcStr] = newOpinion;
        }
    }

    SchedulerOpinion Scheduler::getLatestOpinion(const faabric::Message &msg) {
        const std::string funcStr = funcToString(msg, false);
        return opinionMap[funcStr];
    }

    std::string Scheduler::getBestHostForFunction(const faabric::Message &msg) {
        const std::shared_ptr <spdlog::logger> logger = faabric::utilgetLogger();

        // If we're ignoring the scheduling, just put it on this host regardless
        if (conf.noScheduler == 1) {
            logger->debug("Ignoring scheduler and queueing {} locally", faabric::utilfuncToString(msg, true));
            return thisHost;
        }

        // Accept if we have capacity
        const std::string funcStrNoId = faabric::utilfuncToString(msg, false);
        SchedulerOpinion thisOpinion = opinionMap[funcStrNoId];
        if (thisOpinion == SchedulerOpinion::YES) {
            return thisHost;
        }

        // Get options from the warm set
        std::string warmSet = this->getFunctionWarmSetName(msg);
        redis::Redis &redis = redis::Redis::getQueue();
        std::unordered_set <std::string> warmOptions = redis.smembers(warmSet);

        // Remove this host from the warm options
        warmOptions.erase(thisHost);

        // If we have warm options, pick a random one
        if (!warmOptions.empty()) {
            return faabric::utilrandomStringFromSet(warmOptions);
        }

        // If there are no other warm options and we're a maybe, accept on this host
        if (thisOpinion == SchedulerOpinion::MAYBE) {
            return thisHost;
        }

        // Now there's no warm options we're rejecting, so check all options
        std::unordered_set <std::string> allOptions = redis.smembers(AVAILABLE_HOST_SET);
        allOptions.erase(thisHost);

        if (!allOptions.empty()) {
            // Pick a random option from all hosts
            return faabric::utilrandomStringFromSet(allOptions);
        } else {
            // Give up and try to execute locally
            double inFlightRatio = getFunctionInFlightRatio(msg);
            const std::string oStr = opinionStr(thisOpinion);
            logger->warn("{} overloaded for {}. IF ratio {}, opinion {}", thisHost, funcStrNoId, inFlightRatio,
                         oStr);
            return thisHost;
        }
    }

    void Scheduler::setTestMode(bool val) {
        isTestMode = val;
    }

    std::vector<unsigned int> Scheduler::getRecordedMessagesAll() {
        return recordedMessagesAll;
    }

    std::vector<unsigned int> Scheduler::getRecordedMessagesLocal() {
        return recordedMessagesLocal;
    }

    std::vector<std::pair<std::string, unsigned int>> Scheduler::getRecordedMessagesShared() {
        return recordedMessagesShared;
    }

    bool Scheduler::hasHostCapacity() {
        return _hasHostCapacity;
    }

    void Scheduler::incrementInFlightCount(const faabric::Message &msg) {
        const std::shared_ptr <spdlog::logger> logger = faabric::utilgetLogger();

        // Increment the in-flight count
        const std::string funcStr = faabric::utilfuncToString(msg, false);
        inFlightCountMap[funcStr]++;

        // Check ratios
        double inFlightRatio = getFunctionInFlightRatio(msg);
        int maxInFlightRatio = getFunctionMaxInFlightRatio(msg);
        long nFaaslets = getFunctionWarmFaasletCount(msg);
        logger->debug("{} IF ratio = {} (max {}) faaslets = {}", funcStr, inFlightRatio, maxInFlightRatio, nFaaslets);

        // If we have no faaslets OR if we've got faaslets and are over the in-flight ratio
        // and have capacity, then we need to scale up
        bool needToScale = false;
        needToScale |= (nFaaslets == 0);
        needToScale |= (inFlightRatio > maxInFlightRatio && nFaaslets < conf.maxFaasletsPerFunction);

        if (needToScale) {
            logger->debug("Scaling up {} to {} faaslets", funcStr, nFaaslets + 1);

            // Increment faaslet count here
            incrementWarmFaasletCount(msg);

            // Send bind message (i.e. request a faaslet)
            faabric::Message bindMsg = faabric::utilmessageFactory(msg.user(), msg.function());
            bindMsg.set_type(faabric::Message_MessageType_BIND);
            bindMsg.set_ispython(msg.ispython());
            bindMsg.set_istypescript(msg.istypescript());
            bindMsg.set_pythonuser(msg.pythonuser());
            bindMsg.set_pythonfunction(msg.pythonfunction());

            this->enqueueMessage(bindMsg);
        }

        // Update our opinion
        updateOpinion(msg);
    }

    void Scheduler::decrementInFlightCount(const faabric::Message &msg) {
        const std::string funcStr = faabric::utilfuncToString(msg, false);
        inFlightCountMap[funcStr] = std::max(inFlightCountMap[funcStr] - 1, 0L);

        updateOpinion(msg);
    }

    void Scheduler::incrementWarmFaasletCount(const faabric::Message &msg) {
        std::string funcStr = faabric::utilfuncToString(msg, false);
        faasletCountMap[funcStr]++;

        updateOpinion(msg);
    }

    void Scheduler::decrementWarmFaasletCount(const faabric::Message &msg) {
        const std::string funcStr = faabric::utilfuncToString(msg, false);
        faasletCountMap[funcStr] = std::max(faasletCountMap[funcStr] - 1, 0L);

        updateOpinion(msg);
    }

    std::string Scheduler::getThisHost() {
        return thisHost;
    }

    void Scheduler::broadcastFlush(const faabric::Message &msg) {
        redis::Redis &redis = redis::Redis::getQueue();
        std::unordered_set<std::string> allOptions = redis.smembers(AVAILABLE_HOST_SET);

        faabric::Message newMessage;
        newMessage.set_user(msg.user());
        newMessage.set_function(msg.function());
        newMessage.set_isflushrequest(true);

        // This is pretty inefficient but flush isn't a particularly important operation
        for(auto &otherHost : allOptions) {
            FunctionCallClient c(otherHost);
            c.shareFunctionCall(newMessage);
        }
    }

    void Scheduler::preflightPythonCall() {
        const std::shared_ptr<spdlog::logger> &logger = faabric::utilgetLogger();

        if (conf.pythonPreload != "on") {
            logger->info("Not preloading python runtime");
            return;
        }

        logger->info("Preparing python runtime");

        faabric::Message msg = faabric::utilmessageFactory(PYTHON_USER, PYTHON_FUNC);
        msg.set_ispython(true);
        msg.set_pythonuser("python");
        msg.set_pythonfunction("noop");
        faabric::utilsetMessageId(msg);

        callFunction(msg, true);

        logger->info("Python runtime prepared");
    }

    void Scheduler::setFunctionResult(faabric::Message &msg) {
        redis::Redis &redis = redis::Redis::getQueue();

        // Record which host did the execution
        msg.set_executedhost(faabric::utilgetSystemConfig().endpointHost);

        // Set finish timestamp
        msg.set_finishtimestamp(faabric::utilgetGlobalClock().epochMillis());

        std::string key = msg.resultkey();
        if (key.empty()) {
            throw std::runtime_error("Result key empty. Cannot publish result");
        }

        // Write the successful result to the result queue
        std::vector<uint8_t> inputData = faabric::utilmessageToBytes(msg);
        redis.enqueueBytes(key, inputData);

        // Set the result key to expire
        redis.expire(key, RESULT_KEY_EXPIRY);

        // Set long-lived result for function too
        redis.set(msg.statuskey(), inputData);
        redis.expire(key, STATUS_KEY_EXPIRY);
    }

    faabric::Message Scheduler::getFunctionResult(unsigned int messageId, int timeoutMs) {
        if (messageId == 0) {
            throw std::runtime_error("Must provide non-zero message ID");
        }

        redis::Redis &redis = redis::Redis::getQueue();

        bool isBlocking = timeoutMs > 0;

        std::string resultKey = faabric::utilresultKeyFromMessageId(messageId);

        faabric::Message msgResult;

        if (isBlocking) {
            // Blocking version will throw an exception when timing out which is handled
            // by the caller.
            std::vector<uint8_t> result = redis.dequeueBytes(resultKey, timeoutMs);
            msgResult.ParseFromArray(result.data(), (int) result.size());
        } else {
            // Non-blocking version will tolerate empty responses, therefore we handle
            // the exception here
            std::vector<uint8_t> result;
            try {
                result = redis.dequeueBytes(resultKey, timeoutMs);
            } catch (redis::RedisNoResponseException &ex) {
                // Ok for no response when not blocking
            }

            if (result.empty()) {
                // Empty result has special type
                msgResult.set_type(faabric::Message_MessageType_EMPTY);
            } else {
                // Normal response if we get something from redis
                msgResult.ParseFromArray(result.data(), (int) result.size());
            }
        }

        return msgResult;
    }

    ExecGraph Scheduler::getFunctionExecGraph(unsigned int messageId) {
        ExecGraphNode rootNode = getFunctionExecGraphNode(messageId);
        ExecGraph graph{
                .rootNode = rootNode
        };

        return graph;
    }

    ExecGraphNode Scheduler::getFunctionExecGraphNode(unsigned int messageId) {
        redis::Redis &redis = redis::Redis::getQueue();

        // Get the result for this message
        std::string statusKey = faabric::utilstatusKeyFromMessageId(messageId);
        std::vector<uint8_t> messageBytes = redis.get(statusKey);
        faabric::Message result;
        result.ParseFromArray(messageBytes.data(), (int) messageBytes.size());

        // Recurse through chained calls
        std::unordered_set<unsigned int> chainedMsgIds = getChainedFunctions(messageId);
        std::vector<ExecGraphNode> children;
        for (auto c : chainedMsgIds) {
            children.emplace_back(getFunctionExecGraphNode(c));
        }

        // Build the node
        ExecGraphNode node{
                .msg = result,
                .children = children
        };

        return node;
    }

    std::string Scheduler::getMessageStatus(unsigned int messageId) {
        const faabric::Message result = getFunctionResult(messageId, 0);

        if (result.type() == faabric::Message_MessageType_EMPTY) {
            return "RUNNING";
        } else if (result.returnvalue() == 0) {
            return "SUCCESS: " + result.outputdata();
        } else {
            return "FAILED: " + result.outputdata();
        }
    }

    void Scheduler::logChainedFunction(unsigned int parentMessageId, unsigned int chainedMessageId) {
        redis::Redis &redis = redis::Redis::getQueue();

        const std::string &key = getChainedKey(parentMessageId);
        redis.sadd(key, std::to_string(chainedMessageId));
        redis.expire(key, STATUS_KEY_EXPIRY);
    }

    std::unordered_set<unsigned int> Scheduler::getChainedFunctions(unsigned int msgId) {
        redis::Redis &redis = redis::Redis::getQueue();

        const std::string &key = getChainedKey(msgId);
        const std::unordered_set<std::string> chainedCalls = redis.smembers(key);

        std::unordered_set<unsigned int> chainedIds;
        for (auto i : chainedCalls) {
            chainedIds.insert(std::stoi(i));
        }

        return chainedIds;
    }
}