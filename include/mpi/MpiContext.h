#pragma once

#include <mpi/MpiWorldRegistry.h>
#include <proto/faasm.pb.h>

namespace mpi {
    class MpiContext {
    public:
        MpiContext();

        void createWorld(const message::Message &msg, int worldSize);

        void awaitWorldCreation();

        void joinWorld(const message::Message &msg);

        bool getIsMpi();

        int getWorldId();

        int getRank();
    private:
        bool isMpi;
        int rank;
        int worldId;
    };
}