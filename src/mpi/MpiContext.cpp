#include "mpi/MpiContext.h"

#include <proto/faasm.pb.h>

namespace mpi {
    MpiContext::MpiContext() : isMpi(false), mpiWorldId(-1), mpiRank(-1), worldSize(-1) {

    }

    void MpiContext::create(int worldId, int size) {
        isMpi = true;
        mpiWorldId = worldId;
        mpiRank = 0;
        worldSize = size;

        // TODO - initialise world in global state
    }

    void MpiContext::join(const message::Message &msg) {
        if (!msg.ismpi()) {
            // Not an MPI call
            return;
        }

        isMpi = true;
        mpiWorldId = msg.mpiworldid();
        mpiRank = msg.mpirank();

        // TODO - update from global state

        // TODO - broadcast joining here
    }

    void MpiContext::destroy() {

    }

    void MpiContext::send() {

    }

    void MpiContext::receive() {

    }

    bool MpiContext::getIsMpi() {
        return isMpi;
    }

    int MpiContext::getMpiWorldId() {
        return mpiWorldId;
    }

    int MpiContext::getWorldSize() {
        return worldSize;
    }

    int MpiContext::getMpiRank() {
        return mpiRank;
    }
}