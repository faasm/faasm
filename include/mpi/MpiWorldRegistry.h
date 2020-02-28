#pragma once

#include "mpi/MpiWorld.h"

namespace mpi {
    class MpiWorldRegistry {
    public:
        MpiWorldRegistry() = default;

        mpi::MpiWorld &createWorld(const message::Message &msg, int worldId);

        mpi::MpiWorld &getOrInitialiseWorld(const message::Message &msg, int worldId);

        mpi::MpiWorld &getWorld(int worldId);

    private:
        std::shared_mutex registryMutex;
        std::unordered_map<int, MpiWorld> worldMap;
    };

    MpiWorldRegistry &getMpiWorldRegistry();
}
