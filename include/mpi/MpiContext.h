#pragma once

#include "mpi/MpiWorld.h"

namespace mpi {
    class MpiContext {
    public:
        MpiContext();

        void createWorld(const message::Message &msg, int worldSize);

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