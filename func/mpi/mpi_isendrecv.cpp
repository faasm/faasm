#include <mpi.h>
#include <faasm/faasm.h>


FAASM_MAIN_FUNC() {
    MPI_Init(NULL, NULL);

    int rank;
    int worldSize;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &worldSize);

    // Send messages asynchronously in a ring
    int right = (rank + 1) % worldSize;
    int left = rank - 1;
    if (left < 0) {
        left = worldSize - 1;
    }

    // Asynchronously receive
    size_t msgSize = 10;
    MPI_Request recvRequest;
    int bufferA[msgSize];
    MPI_Irecv(bufferA, msgSize, MPI_INT, left, 123, MPI_COMM_WORLD, &recvRequest);

    // Asynchronously send
    MPI_Request sendRequest;
    int bufferB[msgSize];
    MPI_Isend(bufferB, msgSize, MPI_INT, right, 123, MPI_COMM_WORLD, &sendRequest);

    // Wait for both
    MPI_Wait(&recvRequest, MPI_STATUS_IGNORE);
    MPI_Wait(&sendRequest, MPI_STATUS_IGNORE);

    MPI_Finalize();

    return 0;
}