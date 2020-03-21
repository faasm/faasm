#include <util/locks.h>
#include <util/config.h>

#include "mpi/MpiWorldRegistry.h"

namespace mpi {
    MpiWorldRegistry &getMpiWorldRegistry() {
        static MpiWorldRegistry r;
        return r;
    }

    mpi::MpiWorld &MpiWorldRegistry::createWorld(const message::Message &msg, int worldId) {
        if(worldMap.count(worldId) > 0) {
            throw std::runtime_error("World already exists");
        }

        util::SystemConfig &conf = util::getSystemConfig();
        int worldSize = msg.mpiworldsize();
        if(worldSize <= 0) {
            worldSize = conf.defaultMpiWorldSize;
        }

        util::FullLock lock(registryMutex);
        MpiWorld &world = worldMap[worldId];
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
            throw std::runtime_error("World not initialised: " + std::to_string(worldId));
        }

        return worldMap[worldId];
    }
}