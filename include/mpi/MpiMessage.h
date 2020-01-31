#pragma once

namespace mpi {
    struct MpiMessage {
        int id;
        int sender;
        int destination;
        int type;
        int count;
    };
}