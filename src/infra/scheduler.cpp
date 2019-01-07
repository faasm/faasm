#include "infra.h"

#include "util/util.h"
#include "prof/prof.h"

namespace infra {
    const std::string COUNTER_PREFIX = "n_";
    const std::string WORKER_SET_PREFIX = "w_";
    const std::string PREWARM_QUEUE_PREFIX = "prewarm_";

    Scheduler::Scheduler() = default;

    long Scheduler::getFunctionCount(const message::Message &msg) {
        Redis &redis = Redis::getQueue();
        return redis.getCounter(Scheduler::getFunctionCounterName(msg));
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

        // First of all, send the message to execute the function
        Redis &redis = Redis::getQueue();

        const std::string queueName = getFunctionQueueName(msg);
        logger->debug("Queueing call to {}", infra::funcToString(msg));
        addResultKeyToMessage(msg);
        redis.enqueueMessage(queueName, msg);

        // Then add more workers if necessary
        Scheduler::updateWorkerAllocs(msg);

        return queueName;
    }

    void Scheduler::updateWorkerAllocs(const message::Message &msg, int recursionCount) {
        const std::shared_ptr<spdlog::logger> logger = util::getLogger();

        if(recursionCount > scheduleRecursionLimit) {
            logger->error("Recursion count exceeded (count {})", recursionCount);
            throw std::runtime_error("Scheduling recursion count exceeded");
        }
        
        // NOTE: Function counter will already have been updated
        util::SystemConfig &conf = util::getSystemConfig();

        Redis &redis = Redis::getQueue();

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

        long queueLen = redis.listLength(queueName);
        long workerCount = redis.getCounter(counterKey);

        // See if we're over the queue ratio and have scope to scale up
        bool needMore;
        if(workerCount == 0) {
            needMore = true;
        }
        else {
            double queueRatio = ((double) queueLen) / workerCount;
            needMore = queueRatio > maxQueueRatio;
        }

        logger->debug("{} at length {} with {} workers. Need more: {}", queueName, queueLen, workerCount, needMore);

        if (needMore && workerCount < conf.maxWorkersPerFunction) {
            // Try and get the remote lock for this function, checking that
            // the worker count situation is still the same
            long lockId = redis.acquireConditionalLock(counterKey, workerCount);

            if(lockId <= 0) {
                logger->debug("Failed to get lock for {}. Waiting", counterKey);

                // At this point, the worker count has changed, or we've not got the lock,
                // so we need to wait and try again
                usleep(scheduleWaitMillis * 1000);

                // Recurse
                Scheduler::updateWorkerAllocs(msg, recursionCount + 1);
            } else {
                // Here we have the lock and the worker count is still the same, so
                // we can increment the count and send the bind message
                redis.incr(counterKey);

                // Get the appropriate prewarm queue
                bool affinity = conf.affinity == 1;
                std::string prewarmQueue = Scheduler::getPrewarmQueueForFunction(msg, affinity);

                logger->debug("Requesting bind on {}", prewarmQueue);

                message::Message bindMsg;
                bindMsg.set_type(message::Message_MessageType_BIND);
                bindMsg.set_user(msg.user());
                bindMsg.set_function(msg.function());

                redis.enqueueMessage(prewarmQueue, bindMsg);

                // Release the lock
                redis.releaseLock(counterKey, lockId);
            }
        }
    }

    std::string Scheduler::getPrewarmQueueForFunction(const message::Message &msg, bool affinity) {
        const std::shared_ptr<spdlog::logger> logger = util::getLogger();
        Redis &redis = Redis::getQueue();

        std::string workerSet = Scheduler::getFunctionWorkerSetName(msg);

        std::string workerChoice;
        if(affinity) {
            // Get workers already assigned to this function that are also available
            std::vector<std::string> options = redis.sinter(workerSet, GLOBAL_WORKER_SET);
            if (options.empty()) {
                // If no options, return a random member of the global worker set and
                // add it to the worker set
                workerChoice = redis.srandmember(GLOBAL_WORKER_SET);

                redis.sadd(workerSet, workerChoice);
            } else {
                int idx = util::randomInteger(0, options.size() - 1);
                workerChoice = options.at(idx);
            }
        }
        else {
            // Get workers not already assigned to this function
            std::vector<std::string> options = redis.sdiff(GLOBAL_WORKER_SET, workerSet);
            if(options.empty()) {
                // If no options, return a random member of workers already assigned to func
                workerChoice = redis.srandmember(workerSet);
            }
            else {
                // If options, choose one and add it to the worker's set
                int idx = util::randomInteger(0, options.size() - 1);
                workerChoice = options.at(idx);

                redis.sadd(workerSet, workerChoice);
            }
        }

        // See if we've got any options
        if(workerChoice.empty()) {
            logger->error("No worker host available for scheduling {}", funcToString(msg));
            throw std::runtime_error("No worker host available for scheduling");
        }

        logger->debug("Schedule prewarm on {} for {}", workerChoice, funcToString(msg));

        return Scheduler::getHostPrewarmQueue(workerChoice);
    }

    void Scheduler::workerFinished(const std::string &currentQueue) {
        Redis &redis = Redis::getQueue();

        // Ignore if it's a prewarm queue
        if (util::startsWith(currentQueue, PREWARM_QUEUE_PREFIX)) {
            // Ignore
        }
        else {
            std::string counterName = COUNTER_PREFIX + currentQueue;
            redis.decr(counterName);
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