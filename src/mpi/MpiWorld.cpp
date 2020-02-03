#include "mpi/MpiWorld.h"
#include "mpi/MpiMessage.h"

#include <scheduler/Scheduler.h>
#include <state/State.h>
#include <util/gids.h>
#include <mpi/MpiGlobalBus.h>
#include <util/logging.h>

namespace mpi {
    MpiWorld::MpiWorld() : id(-1), size(-1), thisNodeId(util::getNodeId()) {

    }

    std::string getWorldStateKey(int worldId) {
        if(worldId <= 0) {
            throw std::runtime_error(fmt::format("World ID must be bigger than zero ({})", worldId));
        }
        return "mpi_world_" + std::to_string(worldId);
    }

    std::string getRankStateKey(int worldId, int rankId) {
        if(worldId <= 0 || rankId < 0) {
            throw std::runtime_error(fmt::format("World ID must be >0 and rank ID must be >=0 ({}, {})", worldId, rankId));
        }
        return "mpi_rank_" + std::to_string(worldId) + "_" + std::to_string(rankId);
    }

    std::string getMessageStateKey(int messageId) {
        if(messageId <= 0) {
            throw std::runtime_error(fmt::format("Message ID must be bigger than zero ({})", messageId));
        }
        return "mpi_msg_" + std::to_string(messageId);
    }

    void MpiWorld::setUpStateKV() {
        if (stateKV == nullptr) {
            state::State &state = state::getGlobalState();
            std::string stateKey = getWorldStateKey(id);
            stateKV = state.getKV(user, stateKey, sizeof(MpiWorldState));
        }
    }

    std::shared_ptr<state::StateKeyValue> MpiWorld::getRankNodeState(int rank) {
        state::State &state = state::getGlobalState();
        std::string stateKey = getRankStateKey(id, rank);
        return state.getKV(user, stateKey, NODE_ID_LEN);
    }

    template<typename T>
    std::shared_ptr<state::StateKeyValue> MpiWorld::getMessageState(int messageId, int count) {
        std::string stateKey = getMessageStateKey(messageId);
        state::State &state = state::getGlobalState();
        size_t bufferLen = count * sizeof(T);
        return state.getKV(user, stateKey, bufferLen);
    }

    void MpiWorld::create(const message::Message &call, int newId, int newSize) {
        id = newId;
        user = call.user();
        function = call.function();

        size = newSize;

        // Write this to state
        setUpStateKV();
        pushToState();

        // Register this as the master
        registerRank(0);

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

        for (auto &s: rankNodeMap) {
            getRankNodeState(s.first)->deleteGlobal();
        }
    }

    void MpiWorld::initialiseFromState(const message::Message &msg, int worldId) {
        id = worldId;
        user = msg.user();
        function = msg.function();

        setUpStateKV();

        // Read from state
        MpiWorldState s{};
        stateKV->pull();
        stateKV->get(reinterpret_cast<uint8_t *>(&s));
        size = s.worldSize;
    }

    void MpiWorld::pushToState() {
        // Write to state
        MpiWorldState s{
                .worldSize=this->size,
        };

        stateKV->set(reinterpret_cast<uint8_t *>(&s));
        stateKV->pushFull();
    }

    void MpiWorld::registerRank(int rank) {
        {
            util::FullLock lock(worldMutex);
            rankNodeMap[rank] = thisNodeId;
        }

        // Set the value remotely
        const std::shared_ptr<state::StateKeyValue> &kv = getRankNodeState(rank);
        kv->set(reinterpret_cast<const uint8_t *>(thisNodeId.c_str()));
        kv->pushFull();
    }

    std::string MpiWorld::getNodeForRank(int rank) {
        // Pull from state if not present
        if (rankNodeMap.count(rank) == 0) {
            util::FullLock lock(worldMutex);

            if (rankNodeMap.count(rank) == 0) {
                auto buffer = new uint8_t[NODE_ID_LEN];
                const std::shared_ptr<state::StateKeyValue> &kv = getRankNodeState(rank);
                kv->get(buffer);

                char *bufferChar = reinterpret_cast<char *>(buffer);
                if(bufferChar[0] == '\0') {
                    // No entry for other rank
                    throw std::runtime_error(fmt::format("No node entry for rank {}", rank));
                }

                std::string otherNodeId(bufferChar, bufferChar + NODE_ID_LEN);
                rankNodeMap[rank] = otherNodeId;
            }
        }

        return rankNodeMap[rank];
    }

    template<typename T>
    void MpiWorld::send(int senderRank, int destRank, const T *buffer, int dataType, int count) {
        if(destRank > this->size) {
            throw std::runtime_error(fmt::format("Rank {} bigger than world size {}", destRank, this->size));
        }

        // Generate a message ID
        int msgId = (int) util::generateGid();

        // Write the data to state
        const std::shared_ptr<state::StateKeyValue> &kv = getMessageState<T>(msgId, count);
        kv->set(reinterpret_cast<const uint8_t *>(buffer));

        // Create the message
        auto m = new MpiMessage;
        m->id = msgId;
        m->worldId = id;
        m->sender = senderRank;
        m->destination = destRank;
        m->type = dataType;
        m->count = count;

        // Work out whether the message is sent locally or to another node
        const std::string otherNodeId = getNodeForRank(destRank);
        bool isLocal = otherNodeId == thisNodeId;
        if (isLocal) {
            // Place on relevant in-memory queue
            const std::shared_ptr<InMemoryMpiQueue> &queue = getRankQueue(destRank);
            queue->enqueue(m);
        } else {
            // Push to global state
            kv->pushFull();

            // Send the message
            MpiGlobalBus &bus = mpi::getMpiGlobalBus();
            bus.sendMessageToNode(otherNodeId, m);
        }
    }

    template<typename T>
    void MpiWorld::recv(int destRank, T *buffer, int count) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        // Listen to the in-memory queue for this rank
        const std::shared_ptr<InMemoryMpiQueue> &queue = getRankQueue(destRank);
        const MpiMessage *m = queue->dequeue();

        if (m->count > count) {
            logger->error("Message too long for buffer (msg={}, buffer={})", m->count, count);
            throw std::runtime_error("Message too long");
        }

        const std::shared_ptr<state::StateKeyValue> &kv = getMessageState<T>(m->id, m->count);
        kv->get(reinterpret_cast<uint8_t *>(buffer));
    }

    template void MpiWorld::send<int>(int senderRank, int destRank, const int *buffer, int dataType, int count);

    template void MpiWorld::recv<int>(int destRank, int *buffer, int count);

    void MpiWorld::queueForRank(MpiMessage *msg) {
        if (msg->worldId != id) {
            util::getLogger()->error("Queueing message not meant for this world (msg={}, this={})", msg->worldId, id);
            throw std::runtime_error("Queueing message not for this world");
        }

        int destRank = msg->destination;
        getRankQueue(destRank)->enqueue(msg);
    }

    std::shared_ptr<InMemoryMpiQueue> MpiWorld::getRankQueue(int rank) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        // Check if we know about this rank on this node
        if (rankNodeMap.count(rank) == 0) {
            logger->error("No mapping found for rank {} on this node", rank);
            throw std::runtime_error("No mapping found for rank");
        } else if (rankNodeMap[rank] != thisNodeId) {
            logger->error("Trying to access rank {} on {} but it's on {}", rank, thisNodeId, rankNodeMap[rank]);
            throw std::runtime_error("Accessing in-memory queue for remote rank");
        }

        if (rankQueueMap.count(rank) == 0) {
            util::FullLock lock(worldMutex);

            if (rankQueueMap.count(rank) == 0) {
                auto mq = new InMemoryMpiQueue();
                rankQueueMap.emplace(MpiMessageQueuePair(rank, mq));
            }
        }

        return rankQueueMap[rank];
    }

    long MpiWorld::getRankQueueSize(int rank) {
        const std::shared_ptr<InMemoryMpiQueue> &queue = getRankQueue(rank);
        return queue->size();
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

    void MpiWorld::overrideNodeId(const std::string &newNodeId) {
        thisNodeId = newNodeId;
    }
}