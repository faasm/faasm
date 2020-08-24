#pragma once

#include <scheduler/MpiWorldRegistry.h>
#include <proto/faabric.pb.h>

namespace scheduler {
    class MpiContext {
    public:
        MpiContext();

        void createWorld(const faabric::Message &msg);

        void joinWorld(const faabric::Message &msg);

        bool getIsMpi();

        int getWorldId();

        int getRank();
    private:
        bool isMpi;
        int rank;
        int worldId;
    };
}