#include "mpi/MpiGlobalBus.h"

#define MPI_MESSAGE_TIMEOUT_MS 1000

namespace mpi {
    MpiGlobalBus::MpiGlobalBus() : thisNodeId(util::getNodeId()), redis(redis::Redis::getQueue()) {

    }

    MpiGlobalBus &getMpiGlobalBus() {
        static thread_local MpiGlobalBus mb;

        return mb;
    }

    std::string getMpiQueueNameForNode(const std::string &nodeId) {
        return MPI_QUEUE_PREFIX + nodeId;
    }


    void MpiGlobalBus::sendMessageToNode(const std::string &otherNodeId, MpiMessage *m) {
        std::string queueName = getMpiQueueNameForNode(otherNodeId);
        redis.enqueueBytes(queueName, reinterpret_cast<uint8_t *>(m), sizeof(m));
    }

    MpiMessage *MpiGlobalBus::next(const std::string &otherNodeId) {
        std::string queueName = getMpiQueueNameForNode(otherNodeId);
        auto m = new MpiMessage;
        redis.dequeueBytes(queueName, reinterpret_cast<uint8_t *>(m), sizeof(m), MPI_MESSAGE_TIMEOUT_MS);

        return m;
    }

    MpiMessage *MpiGlobalBus::nextThisNode() {
        return next(thisNodeId);
    }
}