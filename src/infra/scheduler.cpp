#include "infra.h"

#include "util/util.h"
#include "prof/prof.h"

namespace infra {
    const std::string PREWARM_COUNTER = "n_prewarm";
    const std::string COLD_COUNTER = "n_cold";

    const std::string COUNTER_PREFIX = "n_";

    Scheduler::Scheduler() = default;

    long Scheduler::getPrewarmCount() {
        Redis *queue = Redis::getThreadQueue();
        return queue->getCounter(infra::PREWARM_COUNTER);
    }

    long Scheduler::getColdCount() {
        Redis *queue = Redis::getThreadQueue();
        return queue->getCounter(infra::COLD_COUNTER);
    }

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
        if (currentQueue == COLD_QUEUE || currentQueue == PREWARM_QUEUE) {
            return conf.unboundTimeout;
        } else {
            return conf.boundTimeout;
        }
    }

    bool Scheduler::prewarmWorker() {
        Redis *queue = Redis::getThreadQueue();

        util::SystemConfig &conf = util::getSystemConfig();
        bool isPrewarm = queue->incrIfBelowTarget(PREWARM_COUNTER, conf.prewarmTarget);

        if(!isPrewarm) {
            queue->incr(COLD_COUNTER);
        }

        return isPrewarm;
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
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        util::SystemConfig &conf = util::getSystemConfig();

        // Get queue details
        Redis *queue = Redis::getThreadQueue();
        const std::string counter = Scheduler::getFunctionCounterName(msg);
        const std::string queueName = Scheduler::getFunctionQueueName(msg);

        // Get the queue ratio for this function
        int queueRatio = conf.maxQueueRatio;

        // TODO make this configurable
        if(msg.function() == "sgd_step") {
            // No queueing
            queueRatio = 0;
        }

        // Add more workers if necessary
        bool shouldAddWorker = queue->addWorker(counter, queueName, queueRatio, conf.maxWorkersPerFunction);

        // Send bind message to pre-warm queue to enlist help of other workers
        if (shouldAddWorker) {
            Scheduler::sendBindMessage(msg);

            logger->debug(
                    "SCALE-UP {}, max_qr = {}, max_workers = {}",
                    infra::funcToString(msg), queueRatio, conf.maxWorkersPerFunction
            );
        } else {
            logger->debug(
                    "MAINTAIN {}, max_qr = {}, max_workers = {}",
                    infra::funcToString(msg), queueRatio, conf.maxWorkersPerFunction
            );
        }
    }

    void Scheduler::sendBindMessage(const message::Message &msg) {
        // Function counter will already have been updated

        // Decrease prewarm counter
        Redis *queue = Redis::getThreadQueue();
        queue->decr(infra::PREWARM_COUNTER);

        // Send the bind message
        message::Message bindMsg;
        bindMsg.set_type(message::Message_MessageType_BIND);
        bindMsg.set_user(msg.user());
        bindMsg.set_function(msg.function());

        queue->enqueueMessage(PREWARM_QUEUE, bindMsg);

        // Send prewarm message to cold queue to replenish prewarm pool
        message::Message prewarmMsg;
        prewarmMsg.set_type(message::Message_MessageType_PREWARM);
        queue->enqueueMessage(infra::COLD_QUEUE, prewarmMsg);
    }

    std::string Scheduler::workerColdToPrewarm() {
        Redis *queue = Redis::getThreadQueue();
        queue->decr(infra::COLD_COUNTER);
        queue->incr(infra::PREWARM_COUNTER);

        return infra::PREWARM_QUEUE;
    }

    std::string Scheduler::workerPrewarmToBound(const message::Message &msg) {
        // Counters have already been updated and messages sent
        std::string queueName = Scheduler::getFunctionQueueName(msg);
        return queueName;
    }

    void Scheduler::workerFinished(const std::string &currentQueue) {
        Redis *queue = Redis::getThreadQueue();

        if (currentQueue == PREWARM_QUEUE) {
            queue->decr(PREWARM_COUNTER);
        } else if (currentQueue == COLD_QUEUE) {
            queue->decr(COLD_COUNTER);
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