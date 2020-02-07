#pragma once

namespace mpi {
    enum MpiMessageType {
        NORMAL,
        BARRIER_JOIN,
        BARRIER_DONE,
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
}