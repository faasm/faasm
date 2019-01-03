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
        Redis *queue = Redis::getThreadQueue();

        util::SystemConfig &conf = util::getSystemConfig();

        // Work out which workers are in this function set and which are available
        if(conf.affinity == 1) {
            // Get an existing member that's also available for work
            // TODO - implement
            // options = sunion(func_workers, available_workers)
            // if(options) {
            //   selected = random_member(options)
            // else {
            //   selected = srandmember(available_workers)
            // }
        }

        // Send the bind message
        message::Message bindMsg;
        bindMsg.set_type(message::Message_MessageType_BIND);
        bindMsg.set_user(msg.user());
        bindMsg.set_function(msg.function());

        queue->enqueueMessage(PREWARM_QUEUE, bindMsg);
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