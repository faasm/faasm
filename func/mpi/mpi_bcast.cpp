#include <mpi.h>
#include <stdio.h>
#include <faasm/faasm.h>
#include <cstring>
#include <faasm/compare.h>


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

    if (rank == root) {
        memcpy(actual, expected, 4 * sizeof(int));
    }

    // Broadcast (all should subsequently agree)
    MPI_Bcast(actual, 4, MPI_INT, root, MPI_COMM_WORLD);

    if (!faasm::compareIntArrays(actual, expected, 4)) {
        return 1;
    }

    printf("Broadcast succeeded\n");

    MPI_Finalize();

    return 0;
}