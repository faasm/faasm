#pragma once

#include "mpi/MpiMessage.h"

#include <redis/Redis.h>

namespace mpi {
    class MpiGlobalBus {
    public:
        MpiGlobalBus();

        void sendMessageToHost(const std::string &otherHost, MpiMessage *msg);

        MpiMessage *dequeueForHost(const std::string &otherHost);

        void next(const std::string &otherHost);

        long getQueueSize(const std::string &otherHost);
    private:
        std::string thisHost;
        redis::Redis &redis;
    };

    MpiGlobalBus &getMpiGlobalBus();

    std::string getMpiQueueNameForHost(const std::string &host);

}