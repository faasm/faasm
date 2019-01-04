#include "infra.h"

#include "util/util.h"
#include "prof/prof.h"

namespace infra {
    const std::string COUNTER_PREFIX = "n_";
    const std::string WORKER_SET_PREFIX = "w_";
    const std::string PREWARM_QUEUE_PREFIX = "prewarm_";

    Scheduler::Scheduler() = default;

    long Scheduler::getFunctionCount(const message::Message &msg) {
        Redis *redis = Redis::getThreadQueue();
        return redis->getCounter(Scheduler::getFunctionCounterName(msg));
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
        if (util::startsWith(currentQueue, PREWARM_QUEUE_PREFIX)) {
            return conf.unboundTimeout;
        } else {
            return conf.boundTimeout;
        }
    }

    std::string Scheduler::callFunction(message::Message &msg) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        const auto &t = prof::startTimer();

        // First of all, send the message to execute the function
        Redis *redis = Redis::getThreadQueue();

        const std::string queueName = getFunctionQueueName(msg);
        logger->debug("Adding call {} to {}", infra::funcToString(msg), queueName);
        addResultKeyToMessage(msg);
        redis->enqueueMessage(queueName, msg);

        // Then add more workers if necessary
        Scheduler::updateWorkerAllocs(msg);
        prof::logEndTimer("call-function", t);

        return queueName;
    }

    void Scheduler::updateWorkerAllocs(const message::Message &msg) {
        // NOTE: Function counter will already have been updated
        util::SystemConfig &conf = util::getSystemConfig();

        Redis *redis = Redis::getThreadQueue();

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

        long queueLen = redis->listLength(queueName);
        long workerCount = redis->getCounter(counterKey);

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
            long lockId = redis->acquireConditionalLock(counterKey, workerCount);

            if(lockId <= 0) {
                // At this point, the worker count has changed, or we've not got the lock,
                // so we need to wait and try again
                usleep(scheduleWaitMillis * 1000);

                // Recurse
                Scheduler::updateWorkerAllocs(msg);
            } else {
                // Here we have the lock and the worker count is still the same, so
                // we can increment the count and send the bind message
                redis->incr(counterKey);

                // Get the appropriate prewarm queue
                std::string prewarmQueue = Scheduler::getPrewarmQueueForFunction(msg);

                message::Message bindMsg;
                bindMsg.set_type(message::Message_MessageType_BIND);
                bindMsg.set_user(msg.user());
                bindMsg.set_function(msg.function());

                redis->enqueueMessage(prewarmQueue, bindMsg);

                // Release the lock
                redis->releaseLock(counterKey, lockId);
            }
        }
    }

    std::string Scheduler::getPrewarmQueueForFunction(const message::Message &msg) {
        util::SystemConfig &conf = util::getSystemConfig();

        // We only deal in affinity or anti-affinity for now.
        bool affinity = conf.affinity == 1;

        Redis *redis = Redis::getThreadQueue();

        std::string workerSet = Scheduler::getFunctionWorkerSetName(msg);

        std::vector<std::string> options;
        if(affinity) {
            // Get workers already assigned to this function that are also available
            options = redis->sinter(workerSet, GLOBAL_WORKER_SET);
            if (options.empty()) {
                // If no options, return a random member of the global worker set
                std::string workerChoice = redis->srandmember(GLOBAL_WORKER_SET);
                return Scheduler::getHostPrewarmQueue(workerChoice);
            }
        }
        else {
            // Get workers not already assigned to this function
            options = redis->sdiff(GLOBAL_WORKER_SET, workerSet);
            if(options.empty()) {
                // If no options, return a random member of workers already assigned to func
                std::string workerChoice = redis->srandmember(workerSet);
                return Scheduler::getHostPrewarmQueue(workerChoice);
            }
        }

        // Return a random choice from the options
        int idx = util::randomInteger(0, options.size());
        std::string workerChoice = options.at(idx);
        return Scheduler::getHostPrewarmQueue(workerChoice);
    }

    void Scheduler::workerFinished(const std::string &currentQueue) {
        Redis *redis = Redis::getThreadQueue();

        // Ignore if it's a prewarm queue
        if (util::startsWith(currentQueue, PREWARM_QUEUE_PREFIX)) {
            // Ignore
        }
        else {
            std::string counterName = COUNTER_PREFIX + currentQueue;
            redis->decr(counterName);
        }
    }

    std::string Scheduler::getFunctionQueueName(const message::Message &msg) {
        return infra::funcToString(msg);
    }

    std::string Scheduler::getFunctionCounterName(const message::Message &msg) {
        return COUNTER_PREFIX + Scheduler::getFunctionQueueName(msg);
    }

    std::string Scheduler::getFunctionWorkerSetName(const message::Message &msg) {
        return WORKER_SET_PREFIX + Scheduler::getFunctionQueueName(msg);
    }

    std::string Scheduler::getHostName() {
        std::string hostname = util::getEnvVar("HOSTNAME", "");
        return hostname;
    }

    std::string Scheduler::getHostPrewarmQueue(const std::string &hostName) {
        std::string queueName = PREWARM_QUEUE_PREFIX + hostName;

        return queueName;
    }

    std::string Scheduler::getHostPrewarmQueue() {
        return Scheduler::getHostPrewarmQueue(Scheduler::getHostName());
    }
}