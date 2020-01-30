#include <scheduler/Scheduler.h>
#include <state/State.h>
#include "mpi/MpiWorld.h"

namespace mpi {
    MpiWorld::MpiWorld() : worldId(-1), worldSize(-1) {

    }

    void MpiWorld::setUpStateKV() {
        if(stateKV == nullptr) {
            state::State &state = state::getGlobalState();
            stateKey = "mpi_state_" + std::to_string(worldId);
            stateKV = state.getKV(user, stateKey, sizeof(MpiWorldState));
        }
    }

    void MpiWorld::create(const message::Message &call, int id, int size) {
        worldId = id;
        worldSize = size;

        user = call.user();
        function = call.function();

        // Write this to state
        setUpStateKV();
        pushToState();

        // Dispatch all the chained calls
        scheduler::Scheduler &sch = scheduler::getScheduler();
        for (int i = 0; i < worldSize - 1; i++) {
            message::Message msg = util::messageFactory(user, function);
            msg.set_ismpi(true);
            msg.set_mpiworldid(worldId);
            msg.set_mpirank(i + 1);

            sch.callFunction(msg);
        }
    }

    void MpiWorld::initialiseFromState(const message::Message &msg, int id) {
        worldId = id;
        setUpStateKV();

        // Read from state
        MpiWorldState s{};
        stateKV->get(reinterpret_cast<uint8_t *>(&s));

        worldSize = s.worldSize;
    }

    void MpiWorld::pushToState() {
        // Write to state
        MpiWorldState s {
                .worldSize=this->worldSize,
        };

        stateKV->set(reinterpret_cast<uint8_t *>(&s));
    }

    void registerRank(int rank) {
        const std::string nodeId = util::getNodeId();

        // TODO - register locally _and_ remotely
    }

    std::string MpiWorld::getUser() {
        return user;
    }

    std::string MpiWorld::getFunction() {
        return function;
    }

    int MpiWorld::getWorldId() {
        return worldId;
    }

    int MpiWorld::getWorldSize() {
        return worldSize;
    }
}