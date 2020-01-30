#include <scheduler/Scheduler.h>
#include <state/State.h>
#include "mpi/MpiWorld.h"

namespace mpi {
    MpiWorld::MpiWorld() : id(-1), size(-1) {

    }

    void MpiWorld::setUpStateKV() {
        if(stateKV == nullptr) {
            state::State &state = state::getGlobalState();
            std::string stateKey = "mpi_state_" + std::to_string(id);
            stateKV = state.getKV(user, stateKey, sizeof(MpiWorldState));
        }
    }

    std::shared_ptr<state::StateKeyValue> MpiWorld::getRankNodeState(int rank) {
        std::string stateKey = "mpi_" + std::to_string(id) + "_" + std::to_string(rank);
        state::State &state = state::getGlobalState();
        return state.getKV(user, stateKey, NODE_ID_LEN);
    }

    void MpiWorld::create(const message::Message &call, int newId, int newSize) {
        id = newId;
        size = newSize;

        user = call.user();
        function = call.function();

        // Write this to state
        setUpStateKV();
        pushToState();

        // Dispatch all the chained calls
        scheduler::Scheduler &sch = scheduler::getScheduler();
        for (int i = 0; i < size - 1; i++) {
            message::Message msg = util::messageFactory(user, function);
            msg.set_ismpi(true);
            msg.set_mpiworldid(id);
            msg.set_mpirank(i + 1);

            sch.callFunction(msg);
        }
    }

    void MpiWorld::destroy() {
        setUpStateKV();
        stateKV->deleteGlobal();
        
        for(auto &s: rankNodeMap) {
            getRankNodeState(s.first)->deleteGlobal();
        }
    }

    void MpiWorld::initialiseFromState(const message::Message &msg, int worldId) {
        id = worldId;
        setUpStateKV();

        // Read from state
        MpiWorldState s{};
        stateKV->get(reinterpret_cast<uint8_t *>(&s));
        size = s.worldSize;
    }

    void MpiWorld::pushToState() {
        // Write to state
        MpiWorldState s {
                .worldSize=this->size,
        };

        stateKV->set(reinterpret_cast<uint8_t *>(&s));
    }

    void MpiWorld::registerRank(int rank) {
        const std::string nodeId = util::getNodeId();

        util::FullLock lock(worldMutex);
        rankNodeMap[rank] = nodeId;

        // Set the value remotely
        const std::shared_ptr<state::StateKeyValue> &kv = getRankNodeState(rank);
        kv->set(reinterpret_cast<const uint8_t *>(nodeId.c_str()));
    }

    std::string MpiWorld::getNodeForRank(int rank) {
        // Pull from state if not present
        if(rankNodeMap.count(rank) == 0) {
            util::FullLock lock(worldMutex);

            if(rankNodeMap.count(rank) == 0) {
                auto buffer = new uint8_t[NODE_ID_LEN];
                const std::shared_ptr<state::StateKeyValue> &kv = getRankNodeState(rank);
                kv->get(buffer);

                char* bufferChar = reinterpret_cast<char*>(buffer);
                std::string nodeId(bufferChar, bufferChar + NODE_ID_LEN);
                rankNodeMap[rank] = nodeId;
            }
        }

        return rankNodeMap[rank];
    }

    std::string MpiWorld::getUser() {
        return user;
    }

    std::string MpiWorld::getFunction() {
        return function;
    }

    int MpiWorld::getId() {
        return id;
    }

    int MpiWorld::getSize() {
        return size;
    }
}