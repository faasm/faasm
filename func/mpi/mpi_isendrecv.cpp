#include <faasm/faasm.h>
#include <mpi.h>
#include <stdio.h>

FAASM_MAIN_FUNC()
{
    MPI_Init(NULL, NULL);

    int rank;
    int worldSize;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &worldSize);

    // Send and receive messages asynchronously in a ring
    int right = (rank + 1) % worldSize;
    int maxRank = worldSize - 1;
    int left = rank > 0 ? rank - 1 : maxRank;

    // Asynchronously receive from the left
    int recvValue = -1;
    MPI_Request recvRequest;
    MPI_Irecv(&recvValue, 1, MPI_INT, left, 0, MPI_COMM_WORLD, &recvRequest);

    // Asynchronously send to the right
    int sendValue = rank;
    MPI_Request sendRequest;
    MPI_Isend(&sendValue, 1, MPI_INT, right, 0, MPI_COMM_WORLD, &sendRequest);

    // Wait for both
    MPI_Wait(&recvRequest, MPI_STATUS_IGNORE);
    MPI_Wait(&sendRequest, MPI_STATUS_IGNORE);

    // Check the received value is as expected
    if (recvValue != left) {
        printf("Rank %i - async not working properly (got %i expected %i)\n",
               rank,
               recvValue,
               left);
    } else {
        printf("Rank %i - async working properly\n", rank);
    }

    MPI_Finalize();

    return 0;
}