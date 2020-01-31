#include "mpi/MpiContext.h"
#include "mpi/MpiWorldRegistry.h"

#include <util/gids.h>
#include <proto/faasm.pb.h>

namespace mpi {
    MpiContext::MpiContext() : isMpi(false), rank(-1), worldId(-1) {

    }

    void MpiContext::createWorld(const message::Message &msg, int worldSize) {
        // Create the MPI world
        worldId = (int) util::generateGid();
        mpi::MpiWorldRegistry &reg = mpi::getMpiWorldRegistry();
        reg.createWorld(msg, worldId, worldSize);

        // Set up this context
        isMpi = true;
        rank = 0;
    }

    void MpiContext::joinWorld(const message::Message &msg) {
        if (!msg.ismpi()) {
            // Not an MPI call
            return;
        }

        isMpi = true;
        worldId = msg.mpiworldid();
        rank = msg.mpirank();

        // Register with the world
        MpiWorldRegistry &registry = getMpiWorldRegistry();
        MpiWorld &world = registry.getOrInitialiseWorld(msg, worldId);
        world.registerRank(rank);
    }

    bool MpiContext::getIsMpi() {
        return isMpi;
    }

    int MpiContext::getRank() {
        return rank;
    }

    int MpiContext::getWorldId() {
        return worldId;
    }
}