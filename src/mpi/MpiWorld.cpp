#include "mpi/MpiWorld.h"
#include "mpi/MpiMessage.h"

#include <scheduler/Scheduler.h>
#include <state/State.h>
#include <faasmpi/mpi.h>
#include <util/gids.h>
#include <mpi/MpiGlobalBus.h>

namespace mpi {
    MpiWorld::MpiWorld() : id(-1), size(-1), thisNodeId(util::getNodeId()) {

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

    template<typename T>
    std::shared_ptr<state::StateKeyValue> MpiWorld::getMessageState(int messageId, int count) {
        std::string stateKey = "mpi_msg_" + std::to_string(messageId);
        state::State &state = state::getGlobalState();
        size_t bufferLen = count * sizeof(T);
        return state.getKV(user, stateKey, bufferLen);
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

        // TODO - Create an in-memory queue for this rank

        // Set the value remotely
        const std::shared_ptr<state::StateKeyValue> &kv = getRankNodeState(rank);
        kv->set(reinterpret_cast<const uint8_t *>(nodeId.c_str()));
        kv->pushFull();
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

    template<typename T>
    void MpiWorld::send(int sendRank, int destRank, T* buffer, int dataType, int count) {
        const std::string nodeId = getNodeForRank(destRank);

        // Generate a message ID
        int msgId = (int)util::generateGid();

        // Write the data to state
        const std::shared_ptr<state::StateKeyValue> &kv = getMessageState<T>(msgId, count);
        kv->set(buffer);

        // Create the message
        MpiMessage m{
            .id=msgId,
            .sender=sendRank,
            .destination=destRank,
            .type=dataType,
            .count=count,
        };

        // Work out where we're sending it
        if(nodeId == thisNodeId) {
            // Place on relevant in-memory queue if local

        } else {
            // Push state if sending to another node
            kv->pushFull();

            // Send the serialised message
            MpiGlobalBus &bus = mpi::getMpiGlobalBus();

            bus.sendMessageToNode(nodeId, &m);
        }
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