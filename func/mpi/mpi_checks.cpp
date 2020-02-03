#include <mpi.h>
#include <stdio.h>
#include <faasm/faasm.h>

FAASM_MAIN_FUNC() {
    int res = MPI_Init(NULL, NULL);
    if(res != MPI_SUCCESS) {
        printf("Failed on MPI init\n");
        return 1;
    }

    int rank;
    int worldSize;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &worldSize);

    if(rank < 0) {
        printf("Rank must be positive integer or zero (is %i)\n", rank);
        return 1;
    }

    // Check how big the world is
    if (worldSize < 3) {
        printf("World size must be greater than 1 (is %i)\n", worldSize);
        return 1;
    }

    if (rank == 0) {
        // Send messages out to rest of world
        for(int recipientRank = 1; recipientRank < worldSize; recipientRank++) {
            int sentNumber = -100 - recipientRank;
            MPI_Send(&sentNumber, 1, MPI_INT, recipientRank, 0, MPI_COMM_WORLD);
        }

        // Wait for their responses
        for(int recipientRank = 1; recipientRank < worldSize; recipientRank++) {
            int receivedNumber;
            MPI_Recv(&receivedNumber, 1, MPI_INT, recipientRank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            if(receivedNumber != recipientRank) {
                printf("Got unexpected number in master (expected %i but got %i)\n", recipientRank, receivedNumber);
                return 1;
            } else {
                printf("Got expected number from rank %i\n", recipientRank);
            }
        }
    }
    else if (rank > 0) {
        // Check message from master
        int receivedNumber = 0;
        int expectedNumber = -100 - rank;
        MPI_Recv(&receivedNumber, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        if(receivedNumber != expectedNumber) {
            printf("Received unexpected number (got %i, expected %i)\n", receivedNumber, expectedNumber);
        } else {
            printf("Got expected number %i\n", receivedNumber);
        }

        // Send success message back to master
        MPI_Send(&rank, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
    }

    MPI_Finalize();

    return 0;
} 