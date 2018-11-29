#include "infra.h"

#include "util/util.h"
#include "prof/prof.h"

namespace infra {
    const std::string PREWARM_COUNTER = "n_prewarm";
    const std::string COLD_COUNTER = "n_cold";

    const std::string COUNTER_PREFIX = "n_";

    Scheduler::Scheduler() = default;

    long Scheduler::getPrewarmCount(){
        Redis *redis = Redis::getThreadConnection();
        return redis->getCounter(infra::PREWARM_COUNTER);
    }

    long Scheduler::getColdCount() {
        Redis *redis = Redis::getThreadConnection();
        return redis->getCounter(infra::COLD_COUNTER);
    }

    long Scheduler::getFunctionCount(const message::Message &msg) {
        Redis *redis = Redis::getThreadConnection();
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
        util::SystemConfig conf = util::getSystemConfig();
        if (currentQueue == COLD_QUEUE || currentQueue == PREWARM_QUEUE) {
            return conf.unbound_timeout;
        } else {
            return conf.bound_timeout;
        }
    }

    bool Scheduler::isNeedToPrewarm() {
        long prewarmCount = Scheduler::getPrewarmCount();
        util::SystemConfig conf = util::getSystemConfig();
        return prewarmCount < conf.prewarm_target;
    }

    std::string Scheduler::callFunction(message::Message &msg) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        const auto &t = prof::startTimer();

        // First of all, send the message to execute the function
        Redis *redis = Redis::getThreadConnection();

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
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        util::SystemConfig conf = util::getSystemConfig();

        // Get queue details
        Redis *redis = Redis::getThreadConnection();
        const std::string &queueName = Scheduler::getFunctionQueueName(msg);
        const std::string counter = Scheduler::getFunctionCounterName(msg);
        long queueLength = redis->listLength(queueName);
        long workerCount = redis->getCounter(counter);

        // Check whether we need more workers
        double queueRatio = 0;
        bool needsMoreWorkerThreads;
        if (workerCount == 0) {
            logger->debug("Requesting first worker for {}", queueRatio);
            needsMoreWorkerThreads = true;
        } else {
            queueRatio = double(queueLength) / workerCount;
            needsMoreWorkerThreads = queueRatio > conf.max_queue_ratio && workerCount < conf.max_workers_per_function;

            if (needsMoreWorkerThreads) {
                logger->debug("Requesting more workers for {} (queue ratio {})", infra::funcToString(msg), queueRatio);
            } else {
                logger->debug("Not requesting more workers for {} (queue ratio {})", infra::funcToString(msg),
                              queueRatio);
            }
        }

        // Send bind message to pre-warm queue to enlist help of other workers
        if (needsMoreWorkerThreads) {
            Scheduler::sendBindMessage(msg);
        }
    }

    void Scheduler::sendBindMessage(const message::Message &msg) {
        // Increment counter for this function
        const std::string counter = Scheduler::getFunctionCounterName(msg);
        Redis *redis = Redis::getThreadConnection();
        redis->incr(counter);

        // Decrease prewarm counter
        redis->decr(infra::PREWARM_COUNTER);

        // Send the bind message
        message::Message bindMsg;
        bindMsg.set_type(message::Message_MessageType_BIND);
        bindMsg.set_user(msg.user());
        bindMsg.set_function(msg.function());

        redis->enqueueMessage(PREWARM_QUEUE, bindMsg);

        // Send prewarm message to cold queue to replenish prewarm pool
        message::Message prewarmMsg;
        prewarmMsg.set_type(message::Message_MessageType_PREWARM);
        redis->enqueueMessage(infra::COLD_QUEUE, prewarmMsg);
    }

    std::string Scheduler::workerInitialisedCold() {
        Redis *redis = Redis::getThreadConnection();
        redis->incr(infra::COLD_COUNTER);

        return infra::COLD_QUEUE;
    }

    std::string Scheduler::workerInitialisedPrewarm() {
        Redis *redis = Redis::getThreadConnection();
        redis->incr(infra::PREWARM_COUNTER);

        return infra::PREWARM_QUEUE;
    }

    std::string Scheduler::workerColdToPrewarm() {
        Redis *redis = Redis::getThreadConnection();
        redis->decr(infra::COLD_COUNTER);
        redis->incr(infra::PREWARM_COUNTER);

        return infra::PREWARM_QUEUE;
    }

    std::string Scheduler::workerPrewarmToBound(const message::Message &msg) {
        // Counters have already been updated and messages sent

        std::string queueName = Scheduler::getFunctionQueueName(msg);
        return queueName;
    }

    void Scheduler::workerFinished(const std::string &currentQueue) {
        Redis *redis = Redis::getThreadConnection();

        if (currentQueue == PREWARM_QUEUE) {
            redis->decr(PREWARM_COUNTER);
        } else if (currentQueue == COLD_QUEUE) {
            redis->decr(COLD_COUNTER);
        } else {
            std::string counterName = COUNTER_PREFIX + currentQueue;
            redis->decr(COLD_COUNTER);
        }
    }


    std::string Scheduler::getFunctionQueueName(const message::Message &msg) {
        std::string funcQueueName = infra::funcToString(msg);

        return funcQueueName;
    }

    std::string Scheduler::getFunctionCounterName(const message::Message &msg) {
        std::string funcSetName = COUNTER_PREFIX + infra::funcToString(msg);

        return funcSetName;
    }
}