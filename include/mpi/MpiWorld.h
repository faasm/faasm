#pragma once

#include "mpi/MpiMessage.h"

#include <thread>
#include <proto/faasm.pb.h>
#include <state/StateKeyValue.h>
#include <scheduler/InMemoryMessageQueue.h>

namespace mpi {
    typedef util::Queue<MpiMessage *> InMemoryMpiQueue;
    typedef util::Queue<int> InMemoryIntQueue;

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

        std::string getHostForRank(int rank);

        std::string getUser();

        std::string getFunction();

        int getId();

        int getSize();

        void destroy();

        void enqueueMessage(MpiMessage *msg);

        void send(int sendRank, int recvRank,
                  const uint8_t *buffer, faasmpi_datatype_t *dataType, int count,
                  MpiMessageType messageType = MpiMessageType::NORMAL);

        int isend(int sendRank, int recvRank,
                   const uint8_t *buffer, faasmpi_datatype_t *dataType, int count);

        void broadcast(int sendRank,
                       const uint8_t *buffer, faasmpi_datatype_t *dataType, int count,
                       MpiMessageType messageType = MpiMessageType::NORMAL);

        void recv(int sendRank, int recvRank,
                  uint8_t *buffer, faasmpi_datatype_t *dataType, int count,
                  MPI_Status *status, MpiMessageType messageType = MpiMessageType::NORMAL);

        int irecv(int sendRank, int recvRank,
                   uint8_t *buffer, faasmpi_datatype_t *dataType, int count);

        void awaitAsyncRequest(int requestId);

        void scatter(int sendRank, int recvRank,
                     const uint8_t *sendBuffer, faasmpi_datatype_t *sendType, int sendCount,
                     uint8_t *recvBuffer, faasmpi_datatype_t *recvType, int recvCount);

        void gather(int sendRank, int recvRank,
                    const uint8_t *sendBuffer, faasmpi_datatype_t *sendType, int sendCount,
                    uint8_t *recvBuffer, faasmpi_datatype_t *recvType, int recvCount);

        void allGather(int rank, const uint8_t *sendBuffer, faasmpi_datatype_t *sendType, int sendCount,
                       uint8_t *recvBuffer, faasmpi_datatype_t *recvType, int recvCount);

        void reduce(int sendRank, int recvRank, uint8_t *sendBuffer, uint8_t *recvBuffer,
                    faasmpi_datatype_t *datatype, int count, faasmpi_op_t *operation);

        void allReduce(int rank, uint8_t *sendBuffer, uint8_t *recvBuffer, faasmpi_datatype_t *datatype, int count,
                       faasmpi_op_t *operation);

        void allToAll(int rank, uint8_t *sendBuffer, faasmpi_datatype_t *sendType, int sendCount,
                      uint8_t *recvBuffer, faasmpi_datatype_t *recvType, int recvCount);

        void probe(int sendRank, int recvRank, MPI_Status *status);

        void barrier(int thisRank);

        void rmaGet(int sendRank, faasmpi_datatype_t *sendType, int sendCount,
                    uint8_t *recvBuffer, faasmpi_datatype_t *recvType, int recvCount);

        void rmaPut(int sendRank, uint8_t *sendBuffer, faasmpi_datatype_t *sendType, int sendCount,
                    int recvRank, faasmpi_datatype_t *recvType, int recvCount);

        std::shared_ptr<InMemoryMpiQueue> getLocalQueue(int sendRank, int recvRank);

        long getLocalQueueSize(int sendRank, int recvRank);

        void overrideHost(const std::string &newHost);

        void createWindow(const faasmpi_win_t *window, uint8_t *windowPtr);

        void synchronizeRmaWrite(const MpiMessage *msg, bool isRemote);

        double getWTime();

    private:
        int id;
        int size;
        std::string thisHost;
        util::TimePoint creationTime;

        std::shared_mutex worldMutex;

        std::string user;
        std::string function;

        std::shared_ptr<state::StateKeyValue> stateKV;
        std::unordered_map<int, std::string> rankHostMap;

        std::unordered_map<std::string, uint8_t *> windowPointerMap;

        std::unordered_map<std::string, std::shared_ptr<InMemoryMpiQueue>> localQueueMap;
        std::unordered_map<int, std::thread> asyncThreadMap;

        void setUpStateKV();

        std::shared_ptr<state::StateKeyValue> getRankHostState(int rank);

        std::shared_ptr<state::StateKeyValue> getMessageState(int messageId, faasmpi_datatype_t *datatype, int count);

        void checkRankOnThisHost(int rank);

        int doISendRecv(int sendRank, int recvRank, const uint8_t *sendBuffer, uint8_t *recvBuffer,
                         faasmpi_datatype_t *dataType, int count);

        void pushToState();
    };
}
