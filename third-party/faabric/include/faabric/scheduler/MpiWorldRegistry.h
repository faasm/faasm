#pragma once

#include "faabric/scheduler/MpiWorld.h"

namespace faabric::scheduler {
    class MpiWorldRegistry {
    public:
        MpiWorldRegistry() = default;

        scheduler::MpiWorld &createWorld(const faabric::Message &msg, int worldId, std::string hostOverride = "");

        scheduler::MpiWorld &getOrInitialiseWorld(const faabric::Message &msg, int worldId);

        scheduler::MpiWorld &getWorld(int worldId);

        void clear();
    private:
        std::shared_mutex registryMutex;
        std::unordered_map<int, MpiWorld> worldMap;
    };

    MpiWorldRegistry &getMpiWorldRegistry();
}
