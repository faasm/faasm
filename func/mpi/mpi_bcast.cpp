#include <mpi.h>
#include <stdio.h>
#include <faasm/faasm.h>


FAASM_MAIN_FUNC() {
    MPI_Init(NULL, NULL);

    int rank;
    int worldSize;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &worldSize);

    if (worldSize < 3) {
        printf("Need world of size 3 or more\n");
        return 1;
    }

    int root = 2;

    if(rank == root) {
        // Broadcast from the root rank
        int data[4] = {0, 1, 2, 3};
        MPI_Bcast(data, 4, MPI_INT, root, MPI_COMM_WORLD);
    } else {
        // Check others see the message
        int actual[4] = {0, 0, 0, 0};
        MPI_Bcast(actual, 4, MPI_INT, root, MPI_COMM_WORLD);

        if(actual[0] != 0 || actual[1] != 1 || actual[2] != 2 || actual[3] != 3) {
            printf("Broadcast failed (expected [0, 1, 2, 3], got [%i, %i, %i, %i])\n",
                    actual[0], actual[1], actual[2], actual[3]);
            return 1;
        }
    }

    MPI_Finalize();

    return 0;
}