#include <util/locks.h>
#include <util/config.h>
#include <util/logging.h>

#include "scheduler/MpiWorldRegistry.h"

namespace scheduler {
    MpiWorldRegistry &getMpiWorldRegistry() {
        static MpiWorldRegistry r;
        return r;
    }

    scheduler::MpiWorld &MpiWorldRegistry::createWorld(const message::Message &msg, int worldId, std::string hostOverride) {
        if(worldMap.count(worldId) > 0) {
            util::getLogger()->error("World {} already exists", worldId);
            throw std::runtime_error("World already exists");
        }

        util::SystemConfig &conf = util::getSystemConfig();
        int worldSize = msg.mpiworldsize();
        if(worldSize <= 0) {
            worldSize = conf.defaultMpiWorldSize;
        }

        util::FullLock lock(registryMutex);
        MpiWorld &world = worldMap[worldId];

        if(!hostOverride.empty()) {
            world.overrideHost(hostOverride);
        }

        world.create(msg, worldId, worldSize);

        return worldMap[worldId];
    }

    MpiWorld &MpiWorldRegistry::getOrInitialiseWorld(const message::Message &msg, int worldId) {
        // Create world locally if not exists
        if(worldMap.count(worldId) == 0) {
            util::FullLock lock(registryMutex);
            if(worldMap.count(worldId) == 0) {
                MpiWorld &world = worldMap[worldId];
                world.initialiseFromState(msg, worldId);
            }
        }

        return worldMap[worldId];
    }

    MpiWorld &MpiWorldRegistry::getWorld(int worldId) {
        if(worldMap.count(worldId) == 0) {
            util::getLogger()->error("World {} not initialised", worldId);
            throw std::runtime_error("World not initialised");
        }

        return worldMap[worldId];
    }

    void MpiWorldRegistry::clear() {
        util::FullLock lock(registryMutex);
        worldMap.clear();
    }
}