#include <faasm/faasm.h>
#include <mpi.h>
#include <stdio.h>
#include <unistd.h>

FAASM_MAIN_FUNC()
{
    MPI_Init(NULL, NULL);

    int rank;
    int worldSize;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &worldSize);

    if (worldSize < 3) {
        printf("Need world of size 3 or more\n");
        return 1;
    }

    const char* keyA = "rank_1";
    const char* keyB = "rank_2";

    // Have two functions take ages to write some state
    uint8_t data[2] = { (uint8_t)rank, (uint8_t)rank };
    if (rank == 1) {
        sleep(1);
        faasmWriteState(keyA, data, 2);
        faasmPushState(keyA);
    }

    if (rank == 2) {
        sleep(1);
        faasmWriteState(keyB, data, 2);
        faasmPushState(keyB);
    }

    // All workers wait on the barrier
    MPI_Barrier(MPI_COMM_WORLD);

    // Make sure everyone sees the written state
    uint8_t bufferA[2] = { 0, 0 };
    uint8_t bufferB[2] = { 0, 0 };
    faasmPullState(keyA, 2);
    faasmPullState(keyB, 2);
    faasmReadState(keyA, bufferA, 2);
    faasmReadState(keyB, bufferB, 2);

    // Check data is as expected
    if (bufferA[0] != 1 || bufferA[1] != 1 || bufferB[0] != 2 ||
        bufferB[1] != 2) {
        printf("Rank %i got unexpected buffers: [%d, %d] and [%d, %d]\n",
               rank,
               bufferA[0],
               bufferA[1],
               bufferB[0],
               bufferB[1]);
        return 1;
    } else if (rank == 0) {
        printf("Barrier check successful\n");
    }

    MPI_Finalize();

    return 0;
}
