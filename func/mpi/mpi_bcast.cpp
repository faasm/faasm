#include <mpi.h>
#include <stdio.h>
#include <faasm/faasm.h>
#include <cstring>


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
    int expected[4] = {0, 1, 2, 3};
    int actual[4] = {-1, -1, -1, -1};

    if(rank == root) {
        memcpy(actual, expected, 4 * sizeof(int));
    }

    // Broadcast from the root rank
    MPI_Bcast(actual, 4, MPI_INT, root, MPI_COMM_WORLD);

    if (actual[0] != expected[0] || actual[1] != expected[1] ||
        actual[2] != expected[2] || actual[3] != expected[3]) {
        printf("Broadcast failed (expected [%i, %i, %i, %i], got [%i, %i, %i, %i])\n",
               expected[0], expected[1], expected[2], expected[3],
               actual[0], actual[1], actual[2], actual[3]);
        return 1;
    } else if(rank == 0) {
        printf("Broadcast succeeded\n");
    }

    MPI_Finalize();

    return 0;
}