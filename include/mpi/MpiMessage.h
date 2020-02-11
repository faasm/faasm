#pragma once

namespace mpi {
    enum MpiMessageType {
        NORMAL,
        BARRIER_JOIN,
        BARRIER_DONE,
        SCATTER,
        GATHER,
        ALLGATHER,
        REDUCE,
        ALLREDUCE,
    };

    struct MpiMessage {
        int id;
        int worldId;
        int sender;
        int destination;
        int type;
        int count;
        MpiMessageType messageType;
    };

    bool isCollectiveMessage(MpiMessage *m);

    bool isCollectiveMessage(MpiMessageType t);
}