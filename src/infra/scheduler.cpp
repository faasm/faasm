#include "infra.h"

#include "util/util.h"
#include "prof/prof.h"

namespace infra {
    const std::string COUNTER_PREFIX = "n_";

    Scheduler::Scheduler() = default;

    long Scheduler::getFunctionCount(const message::Message &msg) {
        Redis *queue = Redis::getThreadQueue();
        return queue->getCounter(Scheduler::getFunctionCounterName(msg));
    }

    void addResultKeyToMessage(message::Message &msg) {
        // Generate a random result key
        int randomNumber = util::randomInteger();
        std::string resultKey = "Result_";
        resultKey += std::to_string(randomNumber);
        msg.set_resultkey(resultKey);
    }

    int Scheduler::getWorkerTimeout(const std::string &currentQueue) {
        util::SystemConfig &conf = util::getSystemConfig();
        if (currentQueue == PREWARM_QUEUE) {
            return conf.unboundTimeout;
        } else {
            return conf.boundTimeout;
        }
    }

    std::string Scheduler::callFunction(message::Message &msg) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        const auto &t = prof::startTimer();

        // First of all, send the message to execute the function
        Redis *queue = Redis::getThreadQueue();

        const std::string queueName = getFunctionQueueName(msg);
        logger->debug("Adding call {} to {}", infra::funcToString(msg), queueName);
        addResultKeyToMessage(msg);
        queue->enqueueMessage(queueName, msg);

        // Then add more workers if necessary
        Scheduler::updateWorkerAllocs(msg);
        prof::logEndTimer("call-function", t);

        return queueName;
    }

    void Scheduler::updateWorkerAllocs(const message::Message &msg) {
        // NOTE: Function counter will already have been updated
        util::SystemConfig &conf = util::getSystemConfig();

        Redis *queue = Redis::getThreadQueue();

        // Get the max queue ratio for this function
        int maxQueueRatio = conf.maxQueueRatio;

        // TODO make this configurable
        if (msg.function() == "sgd_step") {
            // No queueing
            maxQueueRatio = 0;
        }

        // Get the queue length and worker count
        const std::string queueName = getFunctionQueueName(msg);
        const std::string counterKey = Scheduler::getFunctionCounterName(msg);

        long queueLen = queue->listLength(queueName);
        long workerCount = queue->getCounter(counterKey);

        // See if we're over the queue ratio and have scope to scale up
        bool needMore;
        if(workerCount == 0) {
            needMore = true;
        }
        else {
            double queueRatio = ((double) queueLen) / workerCount;
            needMore = queueRatio > maxQueueRatio;
        }

        if (needMore && workerCount < conf.maxWorkersPerFunction) {
            // Try and get the remote lock for this function, checking that
            // the worker count situation is still the same
            long lockId = queue->acquireConditionalLock(counterKey, workerCount);

            if(lockId <= 0) {
                // At this point, the worker count has changed, or we've not got the lock,
                // so we need to wait and try again
                usleep(scheduleWaitMillis * 1000);

                // Recurse
                Scheduler::updateWorkerAllocs(msg);
            } else {
                // Here we have the lock and the worker count is still the same, so
                // we can increment the count and send the bind message
                queue->incr(counterKey);

                message::Message bindMsg;
                bindMsg.set_type(message::Message_MessageType_BIND);
                bindMsg.set_user(msg.user());
                bindMsg.set_function(msg.function());

                queue->enqueueMessage(PREWARM_QUEUE, bindMsg);

                // Release the lock
                queue->releaseLock(counterKey, lockId);
            }
        }
    }

    void Scheduler::workerFinished(const std::string &currentQueue) {
        Redis *queue = Redis::getThreadQueue();

        if (currentQueue == PREWARM_QUEUE) {
            // Ignore
        } else {
            std::string counterName = COUNTER_PREFIX + currentQueue;
            queue->decr(counterName);
        }
    }


    std::string Scheduler::getFunctionQueueName(const message::Message &msg) {
        return infra::funcToString(msg);
    }

    std::string Scheduler::getFunctionCounterName(const message::Message &msg) {
        return COUNTER_PREFIX + Scheduler::getFunctionQueueName(msg);
    }
}