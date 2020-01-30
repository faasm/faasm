#pragma once

#include <proto/faasm.pb.h>
#include <state/StateKeyValue.h>

namespace mpi {
    struct MpiWorldState {
        int worldSize;
    };

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

        template<typename T>
        void send(int sendRank, int destRank, T *buffer, int dataType, int count);

    private:
        int id;
        int size;
        std::string thisNodeId;

        std::shared_mutex worldMutex;

        std::string user;
        std::string function;

        std::shared_ptr<state::StateKeyValue> stateKV;
        std::unordered_map<int, std::string> rankNodeMap;

        void setUpStateKV();

        std::shared_ptr<state::StateKeyValue> getRankNodeState(int rank);

        template<typename T>
        std::shared_ptr<state::StateKeyValue> getMessageState(int messageId, int count);

        void pushToState();
    };
}
