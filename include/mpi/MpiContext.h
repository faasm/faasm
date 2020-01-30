#pragma once

#include <proto/faasm.pb.h>

namespace mpi {
    class MpiContext {
    public:
        MpiContext();

        void create(const message::Message &call, int worldId, int size);

        void join(const message::Message &call);

        void destroy();

        void send();

        void receive();

        std::string getUser();

        std::string getFunction();

        int getWorldSize();

        bool getIsMpi();

        int getMpiWorldId();

        int getMpiRank();
    private:
        bool isMpi;
        int mpiWorldId;
        int mpiRank;
        int worldSize;

        std::string user;
        std::string function;

        // Cached rank node mappings
        std::unordered_map<int, int> rankNodeMap;
    };
}