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

        void create(const message::Message &call, int worldId, int size);

        void initialiseFromState(const message::Message &msg, int worldId);

        void registerRank(int rank);

        void send();

        void receive();

        std::string getUser();

        std::string getFunction();

        int getWorldId();

        int getWorldSize();

        void destroy();

    private:
        int id;
        int size;

        std::string user;
        std::string function;

        std::string stateKey;
        std::shared_ptr<state::StateKeyValue> stateKV;

        // Cached rank node mappings
        std::unordered_map<int, int> rankNodeMap;

        void setUpStateKV();

        void pushToState();
    };
}
