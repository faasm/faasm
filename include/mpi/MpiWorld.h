#pragma once

#include "mpi/MpiMessage.h"

#include <faasmpi/mpi.h>
#include <proto/faasm.pb.h>
#include <state/StateKeyValue.h>
#include <scheduler/InMemoryMessageQueue.h>

namespace mpi {
    typedef util::Queue<MpiMessage *> InMemoryMpiQueue;
    typedef std::pair<std::string, InMemoryMpiQueue *> MpiMessageQueuePair;

    struct MpiWorldState {
        int worldSize;
    };

    std::string getWorldStateKey(int worldId);

    std::string getRankStateKey(int worldId, int rankId);

    std::string getMessageStateKey(int messageId);

    class MpiWorld {
    public:
        MpiWorld();

        void create(const message::Message &call, int newId, int newSize);

        void initialiseFromState(const message::Message &msg, int worldId);

        void registerRank(int rank);

        std::string getNodeForRank(int rank);

        std::string getUser();

        std::string getFunction();

        int getId();

        int getSize();

        void destroy();

        void enqueueMessage(MpiMessage *msg);

        template<typename T>
        void send(int sendRank, int recvRank, const T *buffer, int dataType, int count,
                  MpiMessageType messageType = MpiMessageType::NORMAL);

        template<typename T>
        void broadcast(int sendRank, const T *buffer, int dataType, int count,
                       MpiMessageType messageType = MpiMessageType::NORMAL);

        template<typename T>
        void recv(int sendRank, int recvRank, T *buffer, int count, MPI_Status *status,
                  MpiMessageType messageType = MpiMessageType::NORMAL);

        template<typename T>
        void scatter(int sendRank, int recvRank,
                    const T* sendBuffer, int sendType, int sendCount,
                    T* recvBuffer, int recvType, int recvCount);

        template<typename T>
        void gather(int sendRank, int recvRank,
                     const T* sendBuffer, int sendType, int sendCount,
                     T* recvBuffer, int recvType, int recvCount);

        template<typename T>
        void allGather(int rank, const T* sendBuffer, int sendType, int sendCount,
                    T* recvBuffer, int recvType, int recvCount);

        void probe(int sendRank, int recvRank, MPI_Status *status);

        void barrier(int thisRank);

        std::shared_ptr<InMemoryMpiQueue> getLocalQueue(int sendRank, int recvRank);

        std::shared_ptr<InMemoryMpiQueue> getCollectiveQueue(int recvRank);

        long getLocalQueueSize(int sendRank, int recvRank);

        void overrideNodeId(const std::string &newNodeId);

    private:
        int id;
        int size;
        std::string thisNodeId;

        std::shared_mutex worldMutex;

        std::string user;
        std::string function;

        std::shared_ptr<state::StateKeyValue> stateKV;
        std::unordered_map<int, std::string> rankNodeMap;

        std::unordered_map<std::string, std::shared_ptr<InMemoryMpiQueue>> localQueueMap;

        std::unordered_map<std::string, std::shared_ptr<InMemoryMpiQueue>> collectiveQueueMap;

        std::shared_ptr<InMemoryMpiQueue> getInMemoryQueue(
                std::unordered_map<std::string, std::shared_ptr<InMemoryMpiQueue>> &queueMap,
                const std::string &key
        );

        void setUpStateKV();

        std::shared_ptr<state::StateKeyValue> getRankNodeState(int rank);

        template<typename T>
        std::shared_ptr<state::StateKeyValue> getMessageState(int messageId, int count);

        void checkRankOnThisNode(int rank);

        void pushToState();
    };
}
