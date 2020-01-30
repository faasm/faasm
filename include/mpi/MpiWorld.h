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

    private:
        int id;
        int size;

        std::shared_mutex worldMutex;

        std::string user;
        std::string function;

        std::string stateKey;
        std::shared_ptr<state::StateKeyValue> stateKV;

        // Cached rank node mappings
        std::unordered_map<int, std::string> rankNodeMap;

        void setUpStateKV();

        void pushToState();
    };
}
