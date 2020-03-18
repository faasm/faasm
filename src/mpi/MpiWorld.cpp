#include "mpi/MpiWorld.h"
#include "mpi/MpiMessage.h"

#include <scheduler/Scheduler.h>
#include <state/State.h>
#include <util/gids.h>
#include <mpi/MpiGlobalBus.h>
#include <util/logging.h>
#include <faasmpi/mpi.h>
#include <util/macros.h>
#include <util/timing.h>


namespace mpi {
    MpiWorld::MpiWorld() : id(-1), size(-1), thisNodeId(util::getNodeId()), creationTime(util::startTimer()) {

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

    std::string getWindowStateKey(int worldId, int rank, size_t size) {
        return "mpi_win_" + std::to_string(worldId) + "_" + std::to_string(rank) + "_" + std::to_string(size);
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

    std::shared_ptr<state::StateKeyValue>
    MpiWorld::getMessageState(int messageId, faasmpi_datatype_t *datatype, int count) {
        std::string stateKey = getMessageStateKey(messageId);
        state::State &state = state::getGlobalState();
        size_t bufferLen = count * datatype->size;
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
    }

    void MpiWorld::initialiseFromState(const message::Message &msg, int worldId) {
        id = worldId;
        user = msg.user();
        function = msg.function();

        setUpStateKV();

        // Read from state
        MpiWorldState s{};
        stateKV->pull();
        stateKV->get(BYTES(&s));
        size = s.worldSize;
    }

    void MpiWorld::pushToState() {
        // Write to state
        MpiWorldState s{
                .worldSize=this->size,
        };

        stateKV->set(BYTES(&s));
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

    void MpiWorld::isend(int sendRank, int recvRank, const uint8_t *buffer, faasmpi_datatype_t *dataType, int count,
                         faasmpi_request_t *request) {
        doISendRecv(sendRank, recvRank, buffer, nullptr, dataType, count, request);
    }

    void MpiWorld::doISendRecv(int sendRank, int recvRank, const uint8_t *sendBuffer, uint8_t *recvBuffer,
                               faasmpi_datatype_t *dataType, int count, faasmpi_request_t *request) {

        bool isSend = sendBuffer != nullptr;

        // Create the pending request object
        request->id = (int) util::generateGid();
        request->sendRank = sendRank;
        request->recvRank = recvRank;
        request->count = count;
        request->dataType = dataType;

        if (isSend) {
            request->sendBuffer = sendBuffer;
            request->type = faasmpi_async_t::ISEND;
        } else {
            request->recvBuffer = recvBuffer;
            request->type = faasmpi_async_t::IRECV;
        }

        // Record this request
        pendingAsyncRequests.insert({request->id, request});

        // Spawn a thread to do the work
        std::thread t([&] {
            // Do the operation (synchronous send/ receive)
            if (isSend) {
                send(request->sendRank, request->recvRank, request->sendBuffer, request->dataType, request->count);
            } else {
                recv(request->sendRank, request->recvRank, request->recvBuffer, request->dataType, request->count,
                     nullptr);
            }

            // Mark the request as completed and notify on the queue
            request->completed = true;
            getLocalAsyncQueue(request->sendRank, request->recvRank)->enqueue(request->id);
        });

        t.detach();
    }

    void MpiWorld::send(int sendRank, int recvRank, const uint8_t *buffer, faasmpi_datatype_t *dataType, int count,
                        MpiMessageType messageType) {
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
        m->type = dataType->id;
        m->count = count;
        m->messageType = messageType;

        // Work out whether the message is sent locally or to another node
        const std::string otherNodeId = getNodeForRank(recvRank);
        bool isLocal = otherNodeId == thisNodeId;

        // Set up message data in state (must obviously be done before dispatching)
        if (count > 0 && buffer != nullptr) {
            const std::shared_ptr<state::StateKeyValue> &kv = getMessageState(msgId, dataType, count);
            kv->set(buffer);

            // Push to global state if not local
            if (!isLocal) {
                kv->pushFull();
            }
        }

        // Dispatch the message locally or globally
        if (isLocal) {
            if (messageType == MpiMessageType::RMA_WRITE) {
                logger->trace("MPI - local RMA write {} -> {}", sendRank, recvRank);
                synchronizeRmaWrite(m, false);
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

    void MpiWorld::broadcast(int sendRank, const uint8_t *buffer, faasmpi_datatype_t *dataType, int count,
                             MpiMessageType messageType) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->trace("MPI - bcast {} -> all", sendRank);

        for (int r = 0; r < size; r++) {
            // Skip this rank (it's doing the broadcasting)
            if (r == sendRank) {
                continue;
            }

            // Send to the other ranks
            send(sendRank, r, buffer, dataType, count, messageType);
        }
    }

    void checkSendRecvMatch(faasmpi_datatype_t *sendType, int sendCount, faasmpi_datatype_t *recvType, int recvCount) {
        if (sendType->id != recvType->id && sendCount == recvCount) {
            const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
            logger->error("Must match type/ count (send {}:{}, recv {}:{})",
                          sendType->id, sendCount, recvType->id, recvCount);
            throw std::runtime_error("Mismatching send/ recv");
        }
    }

    void MpiWorld::scatter(int sendRank, int recvRank,
                           const uint8_t *sendBuffer, faasmpi_datatype_t *sendType, int sendCount,
                           uint8_t *recvBuffer, faasmpi_datatype_t *recvType, int recvCount) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        checkSendRecvMatch(sendType, sendCount, recvType, recvCount);

        size_t sendOffset = sendCount * sendType->size;

        // If we're the sender, do the sending
        if (recvRank == sendRank) {
            logger->trace("MPI - scatter {} -> all", sendRank);

            for (int r = 0; r < size; r++) {
                // Work out the chunk of the send buffer to send to this rank
                const uint8_t *startPtr = sendBuffer + (r * sendOffset);

                if (r == sendRank) {
                    // Copy data directly if this is the send rank
                    const uint8_t *endPtr = startPtr + sendOffset;
                    std::copy(startPtr, endPtr, recvBuffer);
                } else {
                    send(sendRank, r, startPtr, sendType, sendCount, MpiMessageType::SCATTER);
                }
            }
        } else {
            // Do the receiving
            recv(sendRank, recvRank, recvBuffer, recvType, recvCount, nullptr, MpiMessageType::SCATTER);
        }
    }

    void
    MpiWorld::gather(int sendRank, int recvRank, const uint8_t *sendBuffer, faasmpi_datatype_t *sendType, int sendCount,
                     uint8_t *recvBuffer, faasmpi_datatype_t *recvType, int recvCount) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        checkSendRecvMatch(sendType, sendCount, recvType, recvCount);

        size_t sendOffset = sendCount * sendType->size;
        size_t recvOffset = recvCount * recvType->size;

        bool isInPlace = sendBuffer == recvBuffer;

        // If we're the root, do the gathering
        if (sendRank == recvRank) {
            logger->trace("MPI - gather all -> {}", recvRank);

            // Iterate through each rank
            for (int r = 0; r < size; r++) {
                // Work out where in the receive buffer this rank's data goes
                uint8_t *recvChunk = recvBuffer + (r * recvOffset);

                if ((r == recvRank) && isInPlace) {
                    // If operating in-place, data for the root rank is already in position
                    continue;
                } else if (r == recvRank) {
                    // Copy data locally on root
                    std::copy(sendBuffer, sendBuffer + sendOffset, recvChunk);
                } else {
                    // Receive data from rank if it's not the root
                    recv(r, recvRank, recvChunk, recvType, recvCount, nullptr, MpiMessageType::GATHER);
                }
            }
        } else {
            if (isInPlace) {
                // A non-root rank running gather "in place" happens as part of an allgather
                // operation. In this case, the send and receive buffer are the same, and the
                // rank is eventually expecting a broadcast of the gather result into this buffer.
                // This means that this buffer is big enough for the whole gather result, with this
                // rank's data already in place. Therefore we need to send _only_ the part of the send
                // buffer relating to this rank.
                const uint8_t *sendChunk = sendBuffer + (sendRank * sendOffset);
                send(sendRank, recvRank, sendChunk, sendType, sendCount, MpiMessageType::GATHER);
            } else {
                // Normal sending
                send(sendRank, recvRank, sendBuffer, sendType, sendCount, MpiMessageType::GATHER);
            }
        }
    }

    void MpiWorld::allGather(int rank, const uint8_t *sendBuffer, faasmpi_datatype_t *sendType, int sendCount,
                             uint8_t *recvBuffer, faasmpi_datatype_t *recvType, int recvCount) {
        checkSendRecvMatch(sendType, sendCount, recvType, recvCount);

        int root = 0;

        // Do a gather with a hard-coded root
        gather(rank, root, sendBuffer, sendType, sendCount, recvBuffer, recvType, recvCount);

        // Note that sendCount and recvCount here are per-rank, so we need to work out the full buffer size
        int fullCount = recvCount * size;
        if (rank == root) {
            // Broadcast the result
            broadcast(root, recvBuffer, recvType, fullCount, MpiMessageType::ALLGATHER);
        } else {
            // Await the broadcast from the master
            recv(root, rank, recvBuffer, recvType, fullCount, nullptr, MpiMessageType::ALLGATHER);
        }
    }

    void MpiWorld::irecv(int sendRank, int recvRank, uint8_t *buffer, faasmpi_datatype_t *dataType, int count,
                         faasmpi_request_t *request) {
        doISendRecv(sendRank, recvRank, nullptr, buffer, dataType, count, request);
    }

    void MpiWorld::recv(int sendRank, int recvRank,
                        uint8_t *buffer, faasmpi_datatype_t *dataType, int count,
                        MPI_Status *status, MpiMessageType messageType) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        // Listen to the in-memory queue for this rank and message type
        const MpiMessage *m;
        logger->trace("MPI - recv {} -> {}", sendRank, recvRank);
        m = getLocalQueue(sendRank, recvRank)->dequeue();

        if (messageType != m->messageType) {
            logger->error("Message types mismatched on {}->{} (expected={}, got={})", sendRank, recvRank, messageType,
                          m->messageType);
            throw std::runtime_error("Mismatched message types");
        }

        if (m->count > count) {
            logger->error("Message too long for buffer (msg={}, buffer={})", m->count, count);
            throw std::runtime_error("Message too long");
        }

        if (m->count > 0) {
            const std::shared_ptr<state::StateKeyValue> &kv = getMessageState(m->id, dataType, m->count);
            kv->get(buffer);
        }

        // Set status values if required
        if (status != nullptr) {
            status->MPI_SOURCE = m->sender;
            status->MPI_ERROR = MPI_SUCCESS;

            // Note, take the message size here as the receive count may be larger
            status->bytesSize = m->count * dataType->size;

            // TODO - thread through tag
            status->MPI_TAG = -1;
        }
    }

    void MpiWorld::awaitAsyncRequest(faasmpi_request_t *request) {
        // Check if the request is already completed
        bool isCompleted = request->completed;

        // We need to keep dequeueing messages from the async queue until we get
        // to the one we want.
        // Note that we can guarantee this is the only thread _dequeueing_ on this queue
        while (!isCompleted) {
            int msgId = getLocalAsyncQueue(request->sendRank, request->recvRank)->dequeue();
            if (msgId == request->id) {
                isCompleted = true;
            }
        }
    }

    void MpiWorld::reduce(int sendRank, int recvRank, uint8_t *sendBuffer, uint8_t *recvBuffer,
                          faasmpi_datatype_t *datatype, int count, faasmpi_op_t *operation) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        // If we're the receiver, await inputs
        if (sendRank == recvRank) {
            logger->trace("MPI - reduce ({}) all -> {}", operation->id, recvRank);

            size_t bufferSize = datatype->size * count;

            // Zero the receive buffer if we're not operating in-place
            bool isInPlace = sendBuffer == recvBuffer;
            if (!isInPlace) {
                memset(recvBuffer, 0, bufferSize);
            }

            for (int r = 0; r < size; r++) {
                // Work out the data for this rank
                uint8_t *rankData;
                if (r == recvRank && isInPlace) {
                    // If we're receiving from ourselves and in-place, our work is already done
                    // and the results are written in the recv buffer
                    continue;
                } else if (r == recvRank) {
                    // If we're receiving from ourselves not in-place, the data for this rank
                    // is just the send buffer
                    rankData = sendBuffer;
                } else {
                    // If we're receiving from another rank, call recv
                    rankData = new uint8_t[bufferSize];
                    recv(r, recvRank, rankData, datatype, count, nullptr, MpiMessageType::REDUCE);
                }

                if (operation->id == faasmpi_op_sum.id) {
                    if (datatype->id == FAASMPI_INT) {
                        auto recvBufferCast = reinterpret_cast<int *>(recvBuffer);
                        auto rankDataCast = reinterpret_cast<int *>(rankData);

                        for (int slot = 0; slot < count; slot++) {
                            recvBufferCast[slot] += rankDataCast[slot];
                        }
                    } else if (datatype->id == FAASMPI_DOUBLE) {
                        auto recvBufferCast = reinterpret_cast<double *>(recvBuffer);
                        auto rankDataCast = reinterpret_cast<double *>(rankData);

                        for (int slot = 0; slot < count; slot++) {
                            recvBufferCast[slot] += rankDataCast[slot];
                        }
                    } else {
                        throw std::runtime_error("Unsupported type for sum reduction");
                    }
                } else if (operation->id == faasmpi_op_max.id) {
                    if (datatype->id == FAASMPI_INT) {
                        auto recvBufferCast = reinterpret_cast<int *>(recvBuffer);
                        auto rankDataCast = reinterpret_cast<int *>(rankData);

                        for (int slot = 0; slot < count; slot++) {
                            recvBufferCast[slot] = std::max(recvBufferCast[slot], rankDataCast[slot]);
                        }
                    } else if (datatype->id == FAASMPI_DOUBLE) {
                        auto recvBufferCast = reinterpret_cast<double *>(recvBuffer);
                        auto rankDataCast = reinterpret_cast<double *>(rankData);

                        for (int slot = 0; slot < count; slot++) {
                            recvBufferCast[slot] = std::max(recvBufferCast[slot], rankDataCast[slot]);
                        }
                    } else {
                        throw std::runtime_error("Unsupported type for max reduction");
                    }
                } else {
                    throw std::runtime_error("Not yet implemented reduce operation");
                }

                if (r != recvRank) {
                    delete[] rankData;
                }
            }

        } else {
            // Do the sending
            send(sendRank, recvRank, sendBuffer, datatype, count, MpiMessageType::REDUCE);
        }
    }

    void MpiWorld::allReduce(int rank, uint8_t *sendBuffer, uint8_t *recvBuffer, faasmpi_datatype_t *datatype,
                             int count, faasmpi_op_t *operation) {
        // Rank 0 coordinates the allreduce operation
        if (rank == 0) {
            // Run the standard reduce
            reduce(0, 0, sendBuffer, recvBuffer, datatype, count, operation);

            // Broadcast the result
            broadcast(0, recvBuffer, datatype, count, MpiMessageType::ALLREDUCE);
        } else {
            // Run the standard reduce
            reduce(rank, 0, sendBuffer, recvBuffer, datatype, count, operation);

            // Await the broadcast from the master
            recv(0, rank, recvBuffer, datatype, count, nullptr, MpiMessageType::ALLREDUCE);
        }
    }

    void MpiWorld::allToAll(int rank, uint8_t *sendBuffer, faasmpi_datatype_t *sendType, int sendCount,
                            uint8_t *recvBuffer, faasmpi_datatype_t *recvType, int recvCount) {
        checkSendRecvMatch(sendType, sendCount, recvType, recvCount);

        size_t sendOffset = sendCount * sendType->size;

        // Send out messages for this rank
        for (int r = 0; r < size; r++) {
            // Work out what data to send to this rank
            size_t rankOffset = r * sendOffset;
            uint8_t *sendChunk = sendBuffer + rankOffset;

            if (r == rank) {
                // Copy directly
                std::copy(sendChunk, sendChunk + sendOffset, recvBuffer + rankOffset);
            } else {
                // Send message to other rank
                send(rank, r, sendChunk, sendType, sendCount, MpiMessageType::ALLTOALL);
            }
        }

        // Await incoming messages from others
        for (int r = 0; r < size; r++) {
            if (r == rank) {
                continue;
            }

            // Work out where to place the result from this rank
            uint8_t *recvChunk = recvBuffer + (r * sendOffset);

            // Do the receive
            recv(r, rank, recvChunk, recvType, recvCount, nullptr, MpiMessageType::ALLTOALL);
        }
    }

    void MpiWorld::probe(int sendRank, int recvRank, MPI_Status *status) {
        const std::shared_ptr<InMemoryMpiQueue> &queue = getLocalQueue(sendRank, recvRank);
        const MpiMessage *m = queue->peek();

        faasmpi_datatype_t *datatype = getFaasmDatatypeFromId(m->type);
        status->bytesSize = m->count * datatype->size;
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
                recv(r, 0, nullptr, MPI_INT, 0, &s, MpiMessageType::BARRIER_JOIN);
                logger->trace("MPI - recv barrier join {}", s.MPI_SOURCE);
            }

            // Broadcast that the barrier is done
            broadcast(0, nullptr, MPI_INT, 0, MpiMessageType::BARRIER_DONE);
        } else {
            // Tell the root that we're waiting
            logger->trace("MPI - barrier join {}", thisRank);
            send(thisRank, 0, nullptr, MPI_INT, 0, MpiMessageType::BARRIER_JOIN);

            // Receive a message saying the barrier is done
            recv(0, thisRank, nullptr, MPI_INT, 0, nullptr, MpiMessageType::BARRIER_DONE);
            logger->trace("MPI - barrier done {}", thisRank);
        }
    }

    void MpiWorld::enqueueMessage(MpiMessage *msg) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        if (msg->worldId != id) {
            logger->error("Queueing message not meant for this world (msg={}, this={})", msg->worldId, id);
            throw std::runtime_error("Queueing message not for this world");
        }

        if (msg->messageType == MpiMessageType::RMA_WRITE) {
            // NOTE - RMA notifications must be processed synchronously to ensure ordering
            synchronizeRmaWrite(msg, true);
        } else {
            logger->trace("Queueing message locally {} -> {}", msg->sender, msg->destination);
            getLocalQueue(msg->sender, msg->destination)->enqueue(msg);
        }
    }

    std::shared_ptr<InMemoryMpiQueue> MpiWorld::getLocalQueue(int sendRank, int recvRank) {
        checkRankOnThisNode(recvRank);

        std::string key = std::to_string(sendRank) + "_" + std::to_string(recvRank);
        if (localQueueMap.count(key) == 0) {
            util::FullLock lock(worldMutex);

            if (localQueueMap.count(key) == 0) {
                auto mq = new InMemoryMpiQueue();
                localQueueMap.emplace(std::pair<std::string, InMemoryMpiQueue *>(key, mq));
            }
        }

        return localQueueMap[key];
    }

    std::shared_ptr<InMemoryIntQueue> MpiWorld::getLocalAsyncQueue(int sendRank, int recvRank) {
        checkRankOnThisNode(recvRank);

        std::string key = std::to_string(sendRank) + "_" + std::to_string(recvRank);

        if (localAsyncQueueMap.count(key) == 0) {
            util::FullLock lock(worldMutex);

            if (localAsyncQueueMap.count(key) == 0) {
                auto mq = new InMemoryIntQueue();
                localAsyncQueueMap.emplace(std::pair<std::string, InMemoryIntQueue *>(key, mq));
            }
        }

        return localAsyncQueueMap[key];
    }

    void MpiWorld::rmaGet(int sendRank, faasmpi_datatype_t *sendType, int sendCount,
                          uint8_t *recvBuffer, faasmpi_datatype_t *recvType, int recvCount) {
        checkSendRecvMatch(sendType, sendCount, recvType, recvCount);

        // Get the state value that relates to this window
        int buffLen = sendType->size * sendCount;
        const std::string stateKey = getWindowStateKey(id, sendRank, buffLen);
        state::State &state = state::getGlobalState();
        const std::shared_ptr<state::StateKeyValue> &kv = state.getKV(user, stateKey, buffLen);

        // If it's remote, do a pull too
        if (getNodeForRank(sendRank) != thisNodeId) {
            kv->pull();
        }

        // Do the read
        kv->get(recvBuffer);
    }

    void MpiWorld::rmaPut(int sendRank, uint8_t *sendBuffer, faasmpi_datatype_t *sendType, int sendCount,
                          int recvRank, faasmpi_datatype_t *recvType, int recvCount) {
        checkSendRecvMatch(sendType, sendCount, recvType, recvCount);

        // Get the state value for the window to write to
        int buffLen = sendType->size * sendCount;
        const std::string stateKey = getWindowStateKey(id, recvRank, buffLen);
        state::State &state = state::getGlobalState();
        const std::shared_ptr<state::StateKeyValue> &kv = state.getKV(user, stateKey, buffLen);

        // Do the write
        kv->set(sendBuffer);

        // If it's remote, do a push too
        if (getNodeForRank(recvRank) != thisNodeId) {
            kv->pushFull();
        }

        // Notify the receiver of the push
        // NOTE - must specify a count here to say how big the change is
        send(sendRank, recvRank, nullptr, MPI_INT, sendCount, MpiMessageType::RMA_WRITE);
    }

    void MpiWorld::synchronizeRmaWrite(const MpiMessage *msg, bool isRemote) {
        faasmpi_datatype_t *datatype = getFaasmDatatypeFromId(msg->type);
        int winSize = msg->count * datatype->size;
        const std::string key = getWindowStateKey(id, msg->destination, winSize);

        // Get the state KV
        state::State &state = state::getGlobalState();
        const std::shared_ptr<state::StateKeyValue> &kv = state.getKV(user, key, winSize);

        // If remote, pull the state related to the window
        if (isRemote) {
            kv->pull();
        }

        // Write the state to the pointer
        uint8_t *windowPtr = windowPointerMap[key];
        kv->get(windowPtr);
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

    void MpiWorld::createWindow(const faasmpi_win_t *window, uint8_t *windowPtr) {
        const std::string key = getWindowStateKey(id, window->rank, window->size);
        state::State &state = state::getGlobalState();
        const std::shared_ptr<state::StateKeyValue> windowKv = state.getKV(user, key, window->size);

        // Set initial value
        windowKv->set(windowPtr);
        windowKv->pushFull();

        // Add pointer to map
        {
            util::FullLock lock(worldMutex);
            windowPointerMap[key] = windowPtr;
        }
    }

    double MpiWorld::getWTime() {
        double t = util::getTimeDiffMillis(creationTime);
        return t / 1000.0;
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