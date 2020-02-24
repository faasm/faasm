#include <mpi.h>
#include <stdio.h>
#include <faasm/faasm.h>
#include <faasm/print.h>
#include <faasm/compare.h>


FAASM_MAIN_FUNC() {
    MPI_Init(NULL, NULL);

    int rank;
    int worldSize;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &worldSize);

    // Build an array the right size
    int nPerRank = 4;
    int n = worldSize * nPerRank;

    // Set up containers in root
    int *expected = nullptr;
    int *actual = nullptr;
    int root = 2;
    if (rank == root) {
        expected = new int[n];
        for (int i = 0; i < n; i++) {
            expected[i] = i;
        }

        actual = new int[n];
    }

    // Build the data chunk for this rank
    int startIdx = rank * nPerRank;
    int *thisChunk = new int[nPerRank];
    for (int i = 0; i < nPerRank; i++) {
        thisChunk[i] = startIdx + i;
    }

    MPI_Gather(thisChunk, nPerRank, MPI_INT, actual, nPerRank, MPI_INT, root, MPI_COMM_WORLD);

    if (rank == root) {
        if(!faasm::compareArrays<int>(actual, expected, n)) {
            return 1;
        }

        printf("Gather as expected\n");
    }

    delete[] thisChunk;
    delete[] actual;
    delete[] expected;

    MPI_Finalize();

    return 0;
}