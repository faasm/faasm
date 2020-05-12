#include <util/macros.h>
#include "mpi/MpiGlobalBus.h"
#include "mpi/MpiWorldRegistry.h"


#define MPI_MESSAGE_TIMEOUT_MS 1000

namespace mpi {
    MpiGlobalBus::MpiGlobalBus() : thisHost(util::getSystemConfig().endpointHost), redis(redis::Redis::getQueue()) {

    }

    MpiGlobalBus &getMpiGlobalBus() {
        static thread_local MpiGlobalBus mb;

        return mb;
    }

    std::string getMpiQueueNameForHost(const std::string &host) {
        return "mpi_queue_" + host;
    }

    void MpiGlobalBus::sendMessageToHost(const std::string &otherHost, MpiMessage *m) {
        std::string queueName = getMpiQueueNameForHost(otherHost);
        redis.enqueueBytes(queueName, BYTES(m), sizeof(MpiMessage));
    }

    MpiMessage *MpiGlobalBus::dequeueForHost(const std::string &otherHost) {
        std::string queueName = getMpiQueueNameForHost(otherHost);
        auto m = new MpiMessage;
        redis.dequeueBytes(queueName, BYTES(m), sizeof(MpiMessage), MPI_MESSAGE_TIMEOUT_MS);
        
        return m;
    }
    
    void MpiGlobalBus::next(const std::string &otherHost) {
        MpiMessage *m = dequeueForHost(otherHost);

        MpiWorldRegistry &registry = getMpiWorldRegistry();
        MpiWorld &world = registry.getWorld(m->worldId);
        world.enqueueMessage(m);
    }

    long MpiGlobalBus::getQueueSize(const std::string &otherHost) {
        std::string queueName = getMpiQueueNameForHost(otherHost);
        return redis.listLength(queueName);
    }
}
