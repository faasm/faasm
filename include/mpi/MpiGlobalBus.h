#pragma once

#include "mpi/MpiMessage.h"

#include <redis/Redis.h>

namespace mpi {
    class MpiGlobalBus {
    public:
        MpiGlobalBus();

        void sendMessageToNode(const std::string &otherNodeId, MpiMessage *msg);

        MpiMessage *dequeueForNode(const std::string &otherNodeId);

        void next(const std::string &otherNodeId);

        long getQueueSize(const std::string &otherNodeId);
    private:
        std::string thisNodeId;
        redis::Redis &redis;
    };

    MpiGlobalBus &getMpiGlobalBus();

    std::string getMpiQueueNameForNode(const std::string &nodeId);

}