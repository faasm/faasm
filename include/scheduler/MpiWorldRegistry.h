#pragma once

#include "scheduler/MpiWorld.h"

namespace scheduler {
    class MpiWorldRegistry {
    public:
        MpiWorldRegistry() = default;

        scheduler::MpiWorld &createWorld(const message::Message &msg, int worldId);

        scheduler::MpiWorld &getOrInitialiseWorld(const message::Message &msg, int worldId);

        scheduler::MpiWorld &getWorld(int worldId);

    private:
        std::shared_mutex registryMutex;
        std::unordered_map<int, MpiWorld> worldMap;
    };

    MpiWorldRegistry &getMpiWorldRegistry();
}
