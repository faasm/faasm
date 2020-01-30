#pragma once

#include <proto/faasm.pb.h>

namespace mpi {
    class MpiContext {
    public:
        MpiContext();

        void create(int worldId, int size);

        void join(const message::Message &call);

        void destroy();

        void send();

        void receive();

        int getWorldSize();

        bool getIsMpi();

        int getMpiWorldId();

        int getMpiRank();
    private:
        bool isMpi;
        int mpiWorldId;
        int mpiRank;
        int worldSize;

        // Cached rank node mappings
        std::unordered_map<int, int> rankNodeMap;
    };
}