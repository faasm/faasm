#include <mpi.h>
#include <stdio.h>
#include <faasm/faasm.h>
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

    // Set up data in root rank
    int *allData = nullptr;
    int root = 2;
    if (rank == root) {
        allData = new int[n];
        for (int i = 0; i < n; i++) {
            allData[i] = i;
        }
    }

    // Build the expectation
    int startIdx = rank * nPerRank;
    int *expected = new int[nPerRank];
    for (int i = 0; i < nPerRank; i++) {
        expected[i] = startIdx + i;
    }

    // Do the scatter
    int *actual = new int[nPerRank];
    MPI_Scatter(allData, nPerRank, MPI_INT, actual, nPerRank, MPI_INT, root, MPI_COMM_WORLD);

    if (!faasm::compareIntArrays(actual, expected, nPerRank)) {
        return 1;
    }

    printf("Scatter %i: [%i, %i, %i, %i]\n", rank, actual[0], actual[1], actual[2], actual[3]);

    delete[] allData;
    delete[] actual;
    delete[] expected;

    MPI_Finalize();

    return 0;
}