#pragma once

#include "mpi/MpiMessage.h"

#include <redis/Redis.h>

#define MPI_QUEUE_PREFIX "mpi_msg_"

namespace mpi {

    class MpiGlobalBus {
    public:
        MpiGlobalBus();

        void sendMessageToNode(const std::string &otherNodeId, MpiMessage *msg);

        MpiMessage *next(const std::string &otherNodeId);

        MpiMessage *nextThisNode();

    private:
        std::string thisNodeId;
        redis::Redis &redis;
    };

    MpiGlobalBus &getMpiGlobalBus();

    std::string getMpiQueueNameForNode(const std::string &nodeId);
}