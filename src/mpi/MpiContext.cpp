#include "mpi/MpiContext.h"
#include "mpi/MpiWorldRegistry.h"

#include <util/gids.h>
#include <proto/faasm.pb.h>
#include <util/logging.h>
#include <faasmpi/mpi.h>

namespace mpi {
    MpiContext::MpiContext() : isMpi(false), rank(-1), worldId(-1) {

    }

    void MpiContext::createWorld(const message::Message &msg, int worldSize) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        if(msg.mpirank() > 0) {
            logger->error("Attempting to initialise world for non-zero rank {}", msg.mpirank());
            throw std::runtime_error("Initialising world on non-zero rank");
        }

        worldId = (int) util::generateGid();
        logger->debug("Initialising world {}", worldId);

        // Create the MPI world
        mpi::MpiWorldRegistry &reg = mpi::getMpiWorldRegistry();
        reg.createWorld(msg, worldId, worldSize);

        // Set up this context
        isMpi = true;
        rank = 0;
    }

    void MpiContext::awaitWorldCreation() {
        if(rank > 0) {
            throw std::runtime_error(fmt::format("Awaiting world creation on non-zero rank ({})", rank));
        }
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->debug("Awaiting responses from world creation {}", worldId);

        mpi::MpiWorldRegistry &reg = mpi::getMpiWorldRegistry();
        MpiWorld &world = reg.getWorld(worldId);

        // Await messages from world
        for(int senderRank = 1; senderRank <= world.getSize(); senderRank++) {
            world.recv<int>(senderRank, 0, nullptr, 0, nullptr);
        }

        logger->info("Received all creation messages for {}", worldId);
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

        // Notify the master
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->debug("Rank {} joined {}. Sending message to master", rank, worldId);
        int empty[1] = {0};
        world.send<int>(rank, 0, empty, FAASMPI_INT, 0);
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