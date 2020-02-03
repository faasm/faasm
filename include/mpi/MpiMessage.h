#pragma once

namespace mpi {
    struct MpiMessage {
        int id;
        int worldId;
        int sender;
        int destination;
        int type;
        int count;
    };
}