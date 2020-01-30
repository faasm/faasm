#pragma once

#define MPI_STATE_KEY_LEN 20

namespace mpi {
    struct MpiMessage {
        int id;
        int sender;
        int destination;
        int type;
        int count;
        char stateKey[MPI_STATE_KEY_LEN];
    };
}