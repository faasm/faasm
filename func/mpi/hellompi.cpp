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
    int world_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    if(rank == 0) {
        printf("MPI world size %i\n", world_size);
    }

    int message;
    if(rank == 0) {
        message = 123;
    } else {
        message = -1;
    }

    MPI_Bcast(&message, 1, MPI_INT, 0, MPI_COMM_WORLD);
    printf("Rank %i got message %i\n", rank, message);

    MPI_Finalize();

    return 0;
} 