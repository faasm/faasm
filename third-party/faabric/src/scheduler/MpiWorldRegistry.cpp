#include "MpiWorldRegistry.h"

#include <faabric/util/locks.h>
#include <faabric/util/config.h>
#include <faabric/util/logging.h>


namespace faabric::scheduler {
    MpiWorldRegistry &getMpiWorldRegistry() {
        static MpiWorldRegistry r;
        return r;
    }

    scheduler::MpiWorld &MpiWorldRegistry::createWorld(const faabric::Message &msg, int worldId, std::string hostOverride) {
        if(worldMap.count(worldId) > 0) {
            faabric::utilgetLogger()->error("World {} already exists", worldId);
            throw std::runtime_error("World already exists");
        }

        faabric::utilSystemConfig &conf = faabric::utilgetSystemConfig();
        int worldSize = msg.mpiworldsize();
        if(worldSize <= 0) {
            worldSize = conf.defaultMpiWorldSize;
        }

        faabric::utilFullLock lock(registryMutex);
        MpiWorld &world = worldMap[worldId];

        if(!hostOverride.empty()) {
            world.overrideHost(hostOverride);
        }

        world.create(msg, worldId, worldSize);

        return worldMap[worldId];
    }

    MpiWorld &MpiWorldRegistry::getOrInitialiseWorld(const faabric::Message &msg, int worldId) {
        // Create world locally if not exists
        if(worldMap.count(worldId) == 0) {
            faabric::utilFullLock lock(registryMutex);
            if(worldMap.count(worldId) == 0) {
                MpiWorld &world = worldMap[worldId];
                world.initialiseFromState(msg, worldId);
            }
        }

        return worldMap[worldId];
    }

    MpiWorld &MpiWorldRegistry::getWorld(int worldId) {
        if(worldMap.count(worldId) == 0) {
            faabric::utilgetLogger()->error("World {} not initialised", worldId);
            throw std::runtime_error("World not initialised");
        }

        return worldMap[worldId];
    }

    void MpiWorldRegistry::clear() {
        faabric::utilFullLock lock(registryMutex);
        worldMap.clear();
    }
}