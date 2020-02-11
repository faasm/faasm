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
        // NOTE - with the master being rank zero, we want to spawn
        // (size - 1) new functions starting with rank 1
        scheduler::Scheduler &sch = scheduler::getScheduler();
        for (int i = 1; i < size; i++) {
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
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        if (recvRank > this->size - 1) {
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
            if (isCollectiveMessage(m)) {
                logger->trace("MPI - send collective {} -> {}", sendRank, recvRank);
                getCollectiveQueue(recvRank)->enqueue(m);
            } else {
                logger->trace("MPI - send {} -> {}", sendRank, recvRank);
                getLocalQueue(sendRank, recvRank)->enqueue(m);
            }
        } else {
            logger->trace("MPI - send remote {} -> {}", sendRank, recvRank);
            MpiGlobalBus &bus = mpi::getMpiGlobalBus();
            bus.sendMessageToNode(otherNodeId, m);
        }
    }

    template<typename T>
    void MpiWorld::broadcast(int sendRank, const T *buffer, int dataType, int count,
                             MpiMessageType messageType) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->trace("MPI - bcast {} -> all", sendRank);

        for (int r = 0; r < size; r++) {
            // Skip this rank (it's doing the broadcasting)
            if (r == sendRank) {
                continue;
            }

            // Send to the other ranks
            send<T>(sendRank, r, buffer, dataType, count, messageType);
        }
    }

    void checkSendRecvMatch(int sendType, int sendCount, int recvType, int recvCount) {
        if (sendType != recvType && sendCount == recvCount) {
            const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
            logger->error("Must match type/ count (send {}:{}, recv {}:{})",
                          sendType, sendCount, recvType, recvCount);
            throw std::runtime_error("Mismatching send/ recv on scatter");
        }
    }

    template<typename T>
    void MpiWorld::scatter(int sendRank, int recvRank, const T *sendBuffer, int sendType, int sendCount,
                           T *recvBuffer, int recvType, int recvCount) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        checkSendRecvMatch(sendType, sendCount, recvType, recvCount);

        // If we're the sender, do the sending
        if (recvRank == sendRank) {
            logger->trace("MPI - scatter {} -> all", sendRank);

            for (int r = 0; r < size; r++) {
                // Work out the chunk of the send buffer to send to this rank
                const T *startPtr = sendBuffer + (r * sendCount);

                if (r == sendRank) {
                    // Copy data directly if this is the send rank
                    const T *endPtr = startPtr + sendCount;
                    std::copy(startPtr, endPtr, recvBuffer);
                } else {
                    send<T>(sendRank, r, startPtr, sendType, sendCount, MpiMessageType::SCATTER);
                }
            }
        } else {
            // Do the receiving
            recv<T>(sendRank, recvRank, recvBuffer, recvCount, nullptr, MpiMessageType::SCATTER);
        }
    }

    template<typename T>
    void MpiWorld::gather(int sendRank, int recvRank,
                          const T *sendBuffer, int sendType, int sendCount,
                          T *recvBuffer, int recvType, int recvCount) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        checkSendRecvMatch(sendType, sendCount, recvType, recvCount);

        // If we're the receiver, do the gathering
        if (sendRank == recvRank) {
            logger->trace("MPI - gather all -> {}", recvRank);

            for (int r = 0; r < size; r++) {
                // Work out where in the receive buffer this rank's data goes
                T *recvChunk = recvBuffer + (r * recvCount);

                if (r == recvRank) {
                    // Copy data directly if this is the send rank
                    std::copy(sendBuffer, sendBuffer + sendCount, recvChunk);
                } else {
                    recv<T>(r, recvRank, recvChunk, recvCount, nullptr, MpiMessageType::GATHER);
                }
            }
        } else {
            // Do the sending
            send<T>(sendRank, recvRank, sendBuffer, sendType, sendCount, MpiMessageType::GATHER);
        }
    }

    template<typename T>
    void MpiWorld::allGather(int rank, const T *sendBuffer, int sendType, int sendCount,
                             T *recvBuffer, int recvType, int recvCount) {
        checkSendRecvMatch(sendType, sendCount, recvType, recvCount);

        // Note that sendCount and recvCount here are per-rank, so
        // we need to work out the full buffer size
        int fullCount = recvCount * size;

        // Rank 0 coordinates the allgather operation
        if (rank == 0) {
            // Await the incoming messages from all (like a normal gather)
            gather<T>(0, 0, sendBuffer, sendType, sendCount, recvBuffer, recvType, recvCount);

            // Broadcast the result
            broadcast<T>(0, recvBuffer, recvType, fullCount, MpiMessageType::ALLGATHER);
        } else {
            // Send the gather message (recv buffer can be null as it's not needed)
            gather<T>(rank, 0, sendBuffer, sendType, sendCount, nullptr, recvType, recvCount);

            // Await the broadcast from the master
            recv<T>(0, rank, recvBuffer, fullCount, nullptr, MpiMessageType::ALLGATHER);
        }
    }

    template<typename T>
    void MpiWorld::recv(int sendRank, int recvRank, T *buffer, int count, MPI_Status *status,
                        MpiMessageType messageType) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        // Listen to the in-memory queue for this rank and message type
        const MpiMessage *m;
        if (isCollectiveMessage(messageType)) {
            logger->trace("MPI - recv collective {} -> {}", sendRank, recvRank);
            m = getCollectiveQueue(recvRank)->dequeue();
        } else {
            logger->trace("MPI - recv {} -> {}", sendRank, recvRank);
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

    template<typename T>
    void MpiWorld::reduce(int sendRank, int recvRank, T *sendBuffer, T *recvBuffer,
                          int datatype, int count, int operation) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        // If we're the receiver, await inputs
        if (sendRank == recvRank) {
            logger->trace("MPI - reduce ({}) all -> {}", operation, recvRank);

            // Ensure the receive buffer is zeroed
            memset(reinterpret_cast<void *>(recvBuffer), 0, sizeof(T) * count);

            for (int r = 0; r < size; r++) {
                // Create an intermediate for holding the intermediate result
                T *resultBuffer = new T[count];

                // Copy directly or receive from others
                if (r == recvRank) {
                    std::copy(sendBuffer, sendBuffer + count, resultBuffer);
                } else {
                    recv<T>(r, recvRank, resultBuffer, count, nullptr, MpiMessageType::REDUCE);
                }

                if (operation == FAASMPI_OP_SUM) {
                    for (int i = 0; i < count; i++) {
                        recvBuffer[i] += resultBuffer[i];
                    }
                } else {
                    throw std::runtime_error("Not yet implemented reduce operation");
                }

                delete[] resultBuffer;
            }

        } else {
            // Do the sending
            send<T>(sendRank, recvRank, sendBuffer, datatype, count, MpiMessageType::REDUCE);
        }
    }

    template<typename T>
    void MpiWorld::allReduce(int rank, T *sendBuffer, T *recvBuffer, int datatype, int count, int operation) {
        // Rank 0 coordinates the allreduce operation
        if (rank == 0) {
            // Run the standard reduce
            reduce<T>(0, 0, sendBuffer, recvBuffer, datatype, count, operation);

            // Broadcast the result
            broadcast<T>(0, recvBuffer, datatype, count, MpiMessageType::ALLREDUCE);
        } else {
            // Run the standard reduce
            reduce<T>(rank, 0, sendBuffer, nullptr, datatype, count, operation);

            // Await the broadcast from the master
            recv<T>(0, rank, recvBuffer, count, nullptr, MpiMessageType::ALLREDUCE);
        }
    }

    // Concrete template types
    template void MpiWorld::send<int>(int sendRank, int recvRank, const int *buffer, int dataType, int count,
                                      MpiMessageType messageType);

    template void MpiWorld::broadcast<int>(int sendRank, const int *buffer, int dataType, int count,
                                           MpiMessageType messageType);

    template void MpiWorld::recv<int>(int sendRank, int recvRank, int *buffer, int count, MPI_Status *status,
                                      MpiMessageType messageType);

    template void MpiWorld::scatter<int>(int sendRank, int recvRank,
                                         const int *sendBuffer, int sendType, int sendCount,
                                         int *recvBuffer, int recvType, int recvCount);

    template void MpiWorld::gather<int>(int sendRank, int recvRank,
                                        const int *sendBuffer, int sendType, int sendCount,
                                        int *recvBuffer, int recvType, int recvCount);

    template void MpiWorld::allGather<int>(int rank, const int *sendBuffer, int sendType, int sendCount,
                                           int *recvBuffer, int recvType, int recvCount);

    template void MpiWorld::reduce<int>(int sendRank, int recvRank, int *sendBuffer, int *recvBuffer,
                                        int datatype, int count, int operation);

    template void MpiWorld::allReduce<int>(int rank, int *sendBuffer, int *recvBuffer, int datatype,
                                           int count, int operation);


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

    void MpiWorld::barrier(int thisRank) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        if (thisRank == 0) {
            // This is the root, hence just does the waiting

            // Await messages from all others
            for (int r = 1; r < size; r++) {
                MPI_Status s{};
                recv<int>(r, 0, nullptr, 0, &s, MpiMessageType::BARRIER_JOIN);
                logger->trace("MPI - recv barrier join {}", s.MPI_SOURCE);
            }

            // Broadcast that the barrier is done
            broadcast<int>(0, nullptr, FAASMPI_INT, 0, MpiMessageType::BARRIER_DONE);
        } else {
            // Tell the root that we're waiting
            logger->trace("MPI - barrier join {}", thisRank);
            send<int>(thisRank, 0, nullptr, FAASMPI_INT, 0, MpiMessageType::BARRIER_JOIN);

            // Receive a message saying the barrier is done
            recv<int>(0, thisRank, nullptr, FAASMPI_INT, nullptr, MpiMessageType::BARRIER_DONE);
            logger->trace("MPI - barrier done {}", thisRank);
        }
    }

    void MpiWorld::enqueueMessage(MpiMessage *msg) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        if (msg->worldId != id) {
            logger->error("Queueing message not meant for this world (msg={}, this={})", msg->worldId, id);
            throw std::runtime_error("Queueing message not for this world");
        }

        if (isCollectiveMessage(msg)) {
            // Collective message
            logger->trace("Queueing collective locally {} -> {}", msg->sender, msg->destination);
            getCollectiveQueue(msg->destination)->enqueue(msg);
        } else {
            // Normal message
            logger->trace("Queueing normal locally {} -> {}", msg->sender, msg->destination);
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