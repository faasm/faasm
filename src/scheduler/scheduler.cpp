#include "scheduler.h"

#include "util/util.h"
#include "prof/prof.h"

#include "scheduler.h"

#include <util/util.h>

using namespace util;

namespace scheduler {
    const std::string WORKER_SET_PREFIX = "w_";

    Scheduler::Scheduler() : conf(util::getSystemConfig()), redis(redis::Redis::getQueue()) {
        bindQueue = new InMemoryMessageQueue();
    }

    void Scheduler::clear() {
        bindQueue->reset();

        for (const auto &iter: queueMap) {
            delete iter.second;
        }

        queueMap.clear();
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

    void Scheduler::enqueueBindMessage(const message::Message &msg) {
        message::Message bindMsg;
        bindMsg.set_type(message::Message_MessageType_BIND);
        bindMsg.set_user(msg.user());
        bindMsg.set_function(msg.function());

        this->enqueueMessage(bindMsg);
    }

    long Scheduler::getFunctionThreadCount(const message::Message &msg) {
        std::string funcStr = util::funcToString(msg);

        util::SharedLock lock(mx);
        return threadCountMap[funcStr];
    }

    long Scheduler::getFunctionQueueLength(const message::Message &msg) {
        std::string funcStr = util::funcToString(msg);

        InMemoryMessageQueue *q = this->getFunctionQueue(msg);

        util::SharedLock lock(mx);
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
        std::string funcStr = util::funcToString(msg);

        InMemoryMessageQueue *q = this->getFunctionQueue(msg);

        {
            util::FullLock lock(mx);
            threadCountMap[funcStr]++;
        }

        return q;
    }

    void Scheduler::stopListeningToQueue(const message::Message &msg) {
        std::string funcStr = util::funcToString(msg);

        {
            util::FullLock lock(mx);
            threadCountMap[funcStr]--;

            if (threadCountMap[funcStr] == 0) {
                std::string workerSet = this->getFunctionWorkerSetName(msg);
                redis.srem(workerSet, hostname);
            }
        }
    }

    std::string Scheduler::getFunctionWorkerSetName(const message::Message &msg) {
        std::string funcStr = util::funcToString(msg);
        return WORKER_SET_PREFIX + funcStr;
    }

    InMemoryMessageQueue *Scheduler::getBindQueue() {
        return bindQueue;
    }

    MessageQueue &Scheduler::getMessageQueue() {
        return messageQueue;
    }

    Scheduler &getScheduler() {
        static Scheduler scheduler;
        return scheduler;
    }

    void addResultKeyToMessage(message::Message &msg) {
        // Generate a random result key
        int randomNumber = util::randomInteger();
        std::string resultKey = "Result_";
        resultKey += std::to_string(randomNumber);
        msg.set_resultkey(resultKey);
    }

    void Scheduler::placeOnGlobalQueue(message::Message &msg) {
        messageQueue.enqueueMessage(INCOMING_QUEUE, msg);
    }

    std::string Scheduler::callFunction(message::Message &msg) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        if (this->getFunctionThreadCount(msg) > 0) {
            logger->debug("Executing {} locally", util::funcToString(msg));
            addResultKeyToMessage(msg);
            this->enqueueMessage(msg);

            // Add more threads if necessary
            this->updateWorkerAllocs(msg);
        } else {
            bool affinity = conf.affinity == 1;
            std::string bestHost = this->getBestHostForFunction(msg, affinity);

            if (bestHost == hostname) {
                this->enqueueMessage(msg);

                // Send bind message to add a new thread locally
                this->enqueueBindMessage(msg);
            } else {
                // TODO - send call to a different host
            }
        }
    }

    void Scheduler::updateWorkerAllocs(const message::Message &msg, int recursionCount) {
        const std::shared_ptr<spdlog::logger> logger = util::getLogger();

        if (recursionCount > scheduleRecursionLimit) {
            logger->error("Recursion count exceeded (count {})", recursionCount);
            throw std::runtime_error("Scheduling recursion count exceeded");
        }

        // NOTE: Function counter will already have been updated
        // Get the max queue ratio for this function
        int maxQueueRatio = conf.maxQueueRatio;

        // TODO make this configurable
        if (msg.function() == "sgd_step") {
            // No queueing
            maxQueueRatio = 0;
        }

        // Get the queue length and worker count
        const std::string queueName = util::funcToString(msg);
        const std::string counterKey = this->getFunctionCounterName(msg);

        long queueLen = messageQueue.redis.listLength(queueName);
        long workerCount = messageQueue.redis.getCounter(counterKey);

        // See if we're over the queue ratio and have scope to scale up
        bool needMore;
        if (workerCount == 0) {
            needMore = true;
        } else {
            double queueRatio = ((double) queueLen) / workerCount;
            needMore = queueRatio > maxQueueRatio;
        }

        logger->debug("{} at length {} with {} workers. Need more: {}", queueName, queueLen, workerCount, needMore);

        if (needMore && workerCount < conf.maxWorkersPerFunction) {
            // Try and get the remote lock for this function, checking that
            // the worker count situation is still the same
            long lockId = messageQueue.redis.acquireConditionalLock(counterKey, workerCount);

            if (lockId <= 0) {
                logger->debug("Failed to get lock for {}. Waiting", counterKey);

                // At this point, the worker count has changed, or we've not got the lock,
                // so we need to wait and try again
                usleep(scheduleWaitMillis * 1000);

                // Recurse
                this->updateWorkerAllocs(msg, recursionCount + 1);
            } else {
                // Here we have the lock and the worker count is still the same, so
                // we can increment the count and send the bind message
                messageQueue.redis.incr(counterKey);

                // Get the appropriate prewarm queue
                bool affinity = conf.affinity == 1;
                std::string prewarmQueue = this->getBestHostForFunction(msg, affinity);

                logger->debug("Requesting bind on {}", prewarmQueue);

                message::Message bindMsg;
                bindMsg.set_type(message::Message_MessageType_BIND);
                bindMsg.set_user(msg.user());
                bindMsg.set_function(msg.function());

                messageQueue.enqueueMessage(prewarmQueue, bindMsg);

                // Release the lock
                messageQueue.redis.releaseLock(counterKey, lockId);
            }
        }
    }

    std::string Scheduler::getBestHostForFunction(const message::Message &msg, bool affinity) {
        const std::shared_ptr<spdlog::logger> logger = util::getLogger();

        std::string workerSet = this->getFunctionWorkerSetName(msg);

        std::string workerChoice;
        if (affinity) {
            // Get workers already assigned to this function that are also available
            std::vector<std::string> options = messageQueue.redis.sinter(workerSet, GLOBAL_WORKER_SET);
            if (options.empty()) {
                // If no options, return a random member of the global worker set and
                // add it to the worker set
                workerChoice = messageQueue.redis.srandmember(GLOBAL_WORKER_SET);

                messageQueue.redis.sadd(workerSet, workerChoice);
            } else {
                int idx = util::randomInteger(0, options.size() - 1);
                workerChoice = options.at(idx);
            }
        } else {
            // Get workers not already assigned to this function
            std::vector<std::string> options = messageQueue.redis.sdiff(GLOBAL_WORKER_SET, workerSet);
            if (options.empty()) {
                // If no options, return a random member of workers already assigned to func
                workerChoice = messageQueue.redis.srandmember(workerSet);
            } else {
                // If options, choose one and add it to the worker's set
                int idx = util::randomInteger(0, options.size() - 1);
                workerChoice = options.at(idx);

                messageQueue.redis.sadd(workerSet, workerChoice);
            }
        }

        // See if we've got any options
        if (workerChoice.empty()) {
            logger->error("No worker host available for scheduling {}", util::funcToString(msg));
            throw std::runtime_error("No worker host available for scheduling");
        }

        logger->debug("Schedule prewarm on {} for {}", workerChoice, util::funcToString(msg));

        return this->getHostPrewarmQueue(workerChoice);
    }

    void Scheduler::addCurrentHostToWorkerPool() {
        // Add this worker to the list of workers accepting jobs
        hostname = util::getHostName();
        if (hostname.empty()) {
            throw std::runtime_error("HOSTNAME for this machine is empty");
        }

        messageQueue.redis.sadd(GLOBAL_WORKER_SET, hostname);
    }
}