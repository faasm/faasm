#pragma once

#include "mpi/MpiMessage.h"

#include <faasmpi/mpi.h>
#include <proto/faasm.pb.h>
#include <state/StateKeyValue.h>
#include <scheduler/InMemoryMessageQueue.h>

namespace mpi {
    typedef util::Queue<MpiMessage*> InMemoryMpiQueue;
    typedef std::pair<int, InMemoryMpiQueue *> MpiMessageQueuePair;

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

        void queueForRank(MpiMessage *msg);

        template<typename T>
        void send(int senderRank, int destRank, const T *buffer, int dataType, int count);

        template<typename T>
        void recv(int destRank, T *buffer, int count, MPI_Status *status);

        void probe(int destRank, MPI_Status *status);

        std::shared_ptr<InMemoryMpiQueue> getRankQueue(int rank);

        long getRankQueueSize(int rank);

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

        std::unordered_map<int, std::shared_ptr<InMemoryMpiQueue>> rankQueueMap;

        void setUpStateKV();

        std::shared_ptr<state::StateKeyValue> getRankNodeState(int rank);

        template<typename T>
        std::shared_ptr<state::StateKeyValue> getMessageState(int messageId, int count);

        void pushToState();
    };
}
