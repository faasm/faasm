#include "mpi/MpiWorld.h"
#include "mpi/MpiMessage.h"

#include <scheduler/Scheduler.h>
#include <state/State.h>
#include <util/gids.h>
#include <mpi/MpiGlobalBus.h>
#include <util/logging.h>
#include <faasmpi/mpi.h>

namespace mpi {
    MpiWorld::MpiWorld() : id(-1), size(-1), thisNodeId(util::getNodeId()) {

    }

    std::string getWorldStateKey(int worldId) {
        if (worldId <= 0) {
            throw std::runtime_error(fmt::format("World ID must be bigger than zero ({})", worldId));
        }
        return "mpi_world_" + std::to_string(worldId);
    }

    std::string getRankStateKey(int worldId, int rankId) {
        if (worldId <= 0 || rankId < 0) {
            throw std::runtime_error(fmt::format(
                    "World ID must be >0 and rank ID must be >=0 ({}, {})", worldId, rankId
            ));
        }
        return "mpi_rank_" + std::to_string(worldId) + "_" + std::to_string(rankId);
    }

    std::string getMessageStateKey(int messageId) {
        if (messageId <= 0) {
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
        // NOTE - with the master being rank zero, we want to spawn n new
        // functions starting with rank 1
        scheduler::Scheduler &sch = scheduler::getScheduler();
        for (int i = 1; i <= size; i++) {
            message::Message msg = util::messageFactory(user, function);
            msg.set_ismpi(true);
            msg.set_mpiworldid(id);
            msg.set_mpirank(i);

            sch.callFunction(msg);
        }
    }

    void MpiWorld::destroy() {
        setUpStateKV();
        stateKV->deleteGlobal();

        for (auto &s: rankNodeMap) {
            getRankNodeState(s.first)->deleteGlobal();
        }

        localQueueMap.clear();
        collectiveQueueMap.clear();
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
                if (bufferChar[0] == '\0') {
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
    void
    MpiWorld::send(int sendRank, int recvRank, const T *buffer, int dataType, int count, MpiMessageType messageType) {
        if (recvRank > this->size) {
            throw std::runtime_error(fmt::format("Rank {} bigger than world size {}", recvRank, this->size));
        }

        // Generate a message ID
        int msgId = (int) util::generateGid();

        // Create the message
        auto m = new MpiMessage;
        m->id = msgId;
        m->worldId = id;
        m->sender = sendRank;
        m->destination = recvRank;
        m->type = dataType;
        m->count = count;
        m->messageType = messageType;

        // Work out whether the message is sent locally or to another node
        const std::string otherNodeId = getNodeForRank(recvRank);
        bool isLocal = otherNodeId == thisNodeId;

        // Set up message data in state (must obviously be done before dispatching)
        if (count > 0) {
            const std::shared_ptr<state::StateKeyValue> &kv = getMessageState<T>(msgId, count);
            kv->set(reinterpret_cast<const uint8_t *>(buffer));

            // Push to global state if not local
            if (!isLocal) {
                kv->pushFull();
            }
        }

        // Dispatch the message locally or globally
        if (isLocal) {
            const std::shared_ptr<InMemoryMpiQueue> &queue = getLocalQueue(sendRank, recvRank);
            queue->enqueue(m);
        } else {
            MpiGlobalBus &bus = mpi::getMpiGlobalBus();
            bus.sendMessageToNode(otherNodeId, m);
        }
    }

    template<typename T>
    void MpiWorld::broadcast(int sendRank, const T *buffer, int dataType, int count,
            MpiMessageType messageType) {

        for (int r = 0; r <= size; r++) {
            // Skip this rank (it's doing the broadcasting)
            if (r == sendRank) {
                continue;
            }

            // Send to the other ranks
            send<T>(sendRank, r, buffer, dataType, count, messageType);
        }
    }

    template<typename T>
    void MpiWorld::recv(int sendRank, int recvRank, T *buffer, int count, MPI_Status *status,
                        MpiMessageType messageType) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        // Listen to the in-memory queue for this rank and message type
        const MpiMessage *m;
        if (messageType == MpiMessageType::BARRIER_JOIN ||
            messageType == MpiMessageType::BARRIER_DONE) {
            m = getCollectiveQueue(recvRank)->dequeue();
        } else {
            m = getLocalQueue(sendRank, recvRank)->dequeue();
        }

        if (m->count > count) {
            logger->error("Message too long for buffer (msg={}, buffer={})", m->count, count);
            throw std::runtime_error("Message too long");
        }

        if (m->count > 0) {
            const std::shared_ptr<state::StateKeyValue> &kv = getMessageState<T>(m->id, m->count);
            kv->get(reinterpret_cast<uint8_t *>(buffer));
        }

        // Set status values if required
        if (status != nullptr) {
            status->MPI_SOURCE = m->sender;
            status->MPI_ERROR = MPI_SUCCESS;

            // Note, take the message size here as the receive count may be larger
            status->bytesSize = m->count * sizeof(T);

            // TODO - thread through tag
            status->MPI_TAG = -1;
        }
    }

    template void MpiWorld::send<int>(int sendRank, int recvRank, const int *buffer, int dataType, int count,
                                      MpiMessageType messageType);

    template void MpiWorld::broadcast<int>(int sendRank, const int *buffer, int dataType, int count,
                                           MpiMessageType messageType);

    template void MpiWorld::recv<int>(int sendRank, int recvRank, int *buffer, int count, MPI_Status *status,
                                      MpiMessageType messageType);

    void MpiWorld::probe(int sendRank, int recvRank, MPI_Status *status) {
        const std::shared_ptr<InMemoryMpiQueue> &queue = getLocalQueue(sendRank, recvRank);
        const MpiMessage *m = queue->peek();

        if (m->type == FAASMPI_INT) {
            status->bytesSize = m->count * sizeof(int);
        } else {
            throw std::runtime_error(fmt::format("Not yet implemented message datatype {}", m->type));
        }

        status->MPI_ERROR = 0;
        status->MPI_SOURCE = m->sender;
    }

    void MpiWorld::barrier(int recvRank) {
        if (recvRank == 0) {
            // We're on the root, we're the one doing the waiting

            // Await messages from all others
            for (int r = 1; r <= size; r++) {
                recv<int>(r, 0, nullptr, 0, nullptr);
            }

            // Broadcast that the barrier is done
            broadcast<int>(0, nullptr, FAASMPI_INT, 0, MpiMessageType::BARRIER_DONE);
        } else {
            // Tell the root that we're waiting
            send<int>(0, recvRank, nullptr, FAASMPI_INT, 0, MpiMessageType::BARRIER_JOIN);

            // Receive a message on the collective queue from neighbour
            recv<int>(0, recvRank, nullptr, FAASMPI_INT, 0, MpiMessageType::BARRIER_DONE);
        }
    }

    void MpiWorld::enqueueMessage(MpiMessage *msg) {
        if (msg->worldId != id) {
            util::getLogger()->error("Queueing message not meant for this world (msg={}, this={})", msg->worldId, id);
            throw std::runtime_error("Queueing message not for this world");
        }

        if (msg->messageType == MpiMessageType::BARRIER_JOIN ||
            msg->messageType == MpiMessageType::BARRIER_DONE) {
            // Collective message
            getCollectiveQueue(msg->destination)->enqueue(msg);
        } else {
            // Normal message
            getLocalQueue(msg->sender, msg->destination)->enqueue(msg);
        }
    }

    std::shared_ptr<InMemoryMpiQueue> MpiWorld::getLocalQueue(int sendRank, int recvRank) {
        checkRankOnThisNode(recvRank);

        std::string key = std::to_string(sendRank) + "_" + std::to_string(recvRank);
        return getInMemoryQueue(localQueueMap, key);
    }

    std::shared_ptr<InMemoryMpiQueue> MpiWorld::getCollectiveQueue(int recvRank) {
        checkRankOnThisNode(recvRank);

        std::string key = "collective_" + std::to_string(recvRank);
        return getInMemoryQueue(collectiveQueueMap, key);
    }

    std::shared_ptr<InMemoryMpiQueue> MpiWorld::getInMemoryQueue(
            std::unordered_map<std::string, std::shared_ptr<InMemoryMpiQueue>> &queueMap,
            const std::string &key
    ) {
        if (queueMap.count(key) == 0) {
            util::FullLock lock(worldMutex);

            if (queueMap.count(key) == 0) {
                auto mq = new InMemoryMpiQueue();
                queueMap.emplace(MpiMessageQueuePair(key, mq));
            }
        }

        return queueMap[key];
    }

    long MpiWorld::getLocalQueueSize(int sendRank, int recvRank) {
        const std::shared_ptr<InMemoryMpiQueue> &queue = getLocalQueue(sendRank, recvRank);
        return queue->size();
    }

    void MpiWorld::checkRankOnThisNode(int rank) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        // Check if we know about this rank on this node
        if (rankNodeMap.count(rank) == 0) {
            logger->error("No mapping found for rank {} on this node", rank);
            throw std::runtime_error("No mapping found for rank");
        } else if (rankNodeMap[rank] != thisNodeId) {
            logger->error("Trying to access rank {} on {} but it's on {}", rank, thisNodeId, rankNodeMap[rank]);
            throw std::runtime_error("Accessing in-memory queue for remote rank");
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

    void MpiWorld::overrideNodeId(const std::string &newNodeId) {
        thisNodeId = newNodeId;
    }
}