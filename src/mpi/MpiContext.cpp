#include "mpi/MpiContext.h"

#include <proto/faasm.pb.h>
#include <util/func.h>
#include <scheduler/Scheduler.h>

namespace mpi {
    MpiContext::MpiContext() : isMpi(false), mpiWorldId(-1), mpiRank(-1), worldSize(-1) {

    }

    void MpiContext::create(const message::Message &call, int worldId, int size) {
        isMpi = true;
        mpiWorldId = worldId;
        mpiRank = 0;
        worldSize = size;

        user = call.user();
        function = call.function();

        scheduler::Scheduler &sch = scheduler::getScheduler();

        // Dispatch all the chained calls
        for (int i = 0; i < worldSize - 1; i++) {
            message::Message msg = util::messageFactory(user, function);
            msg.set_ismpi(true);
            msg.set_mpiworldid(worldId);
            msg.set_mpirank(i + 1);

            sch.callFunction(msg);
        }
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

    std::string MpiContext::getUser() {
        return user;
    }

    std::string MpiContext::getFunction() {
        return function;
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