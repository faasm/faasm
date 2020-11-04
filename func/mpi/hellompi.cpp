#include <faasm/faasm.h>
#include <mpi.h>
#include <stdio.h>

FAASM_MAIN_FUNC()
{
    int res = MPI_Init(NULL, NULL);
    if (res != MPI_SUCCESS) {
        printf("Failed on MPI init\n");
        return 1;
    }

    int rank;
    int worldSize;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &worldSize);

    if (rank == 0) {
        printf("MPI world size %i\n", worldSize);
    }

    int message;
    if (rank == 0) {
        message = 123;
    } else {
        message = -1;
    }

    MPI_Bcast(&message, 1, MPI_INT, 0, MPI_COMM_WORLD);
    printf("Rank %i got message %i\n", rank, message);

    MPI_Finalize();

    return 0;
}