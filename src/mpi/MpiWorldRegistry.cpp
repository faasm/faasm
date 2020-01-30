#include <util/locks.h>

#include "mpi/MpiWorldRegistry.h"

namespace mpi {
    MpiWorldRegistry &getMpiWorldRegistry() {
        static MpiWorldRegistry r;
        return r;
    }

    mpi::MpiWorld &MpiWorldRegistry::createWorld(const message::Message &msg, int worldId, int size) {
        if(worldMap.count(worldId) > 0) {
            throw std::runtime_error("World already exists");
        }

        util::FullLock lock(registryMutex);
        MpiWorld &world = worldMap[worldId];
        world.create(msg, worldId, size);

        return worldMap[worldId];
    }

    MpiWorld &MpiWorldRegistry::getWorld(const message::Message &msg, int worldId) {
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
}