#include "mpi/MpiGlobalBus.h"
#include "mpi/MpiWorldRegistry.h"


#define MPI_MESSAGE_TIMEOUT_MS 1000

namespace mpi {
    MpiGlobalBus::MpiGlobalBus() : thisNodeId(util::getNodeId()), redis(redis::Redis::getQueue()) {

    }

    MpiGlobalBus &getMpiGlobalBus() {
        static thread_local MpiGlobalBus mb;

        return mb;
    }

    std::string getMpiQueueNameForNode(const std::string &nodeId) {
        return "mpi_queue_" + nodeId;
    }


    void MpiGlobalBus::sendMessageToNode(const std::string &otherNodeId, MpiMessage *m) {
        std::string queueName = getMpiQueueNameForNode(otherNodeId);
        redis.enqueueBytes(queueName, reinterpret_cast<uint8_t *>(m), sizeof(MpiMessage));
    }

    MpiMessage *MpiGlobalBus::dequeueForNode(const std::string &otherNodeId) {
        std::string queueName = getMpiQueueNameForNode(otherNodeId);
        auto m = new MpiMessage;
        redis.dequeueBytes(queueName, reinterpret_cast<uint8_t *>(m), sizeof(MpiMessage), MPI_MESSAGE_TIMEOUT_MS);
        
        return m;
    }
    
    void MpiGlobalBus::next(const std::string &otherNodeId) {
        MpiMessage *m = dequeueForNode(otherNodeId);

        MpiWorldRegistry &registry = getMpiWorldRegistry();
        MpiWorld &world = registry.getWorld(m->worldId);
        world.enqueueMessage(m);
    }

    long MpiGlobalBus::getQueueSize(const std::string &otherNodeId) {
        std::string queueName = getMpiQueueNameForNode(otherNodeId);
        return redis.listLength(queueName);
    }
}
