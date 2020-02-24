#include <mpi.h>
#include <stdio.h>
#include <faasm/faasm.h>
#include <string.h>
#include <faasm/compare.h>


FAASM_MAIN_FUNC() {
    MPI_Init(NULL, NULL);

    int rank;
    int worldSize;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &worldSize);

    // Create an array of three numbers on this process
    int numsThisProc[3] = {rank, 10 * rank, 100 * rank};

    int root = 1;
    int *expected = nullptr;
    int *result = nullptr;

    if (rank == root) {
        // Build expectation
        expected = new int[3];
        memset(expected, 0, 3 * sizeof(int));

        for (int r = 0; r < worldSize; r++) {
            expected[0] += r;
            expected[1] += 10 * r;
            expected[2] += 100 * r;
        }

        // Get the result on the root
        result = new int[3];
    }

    // Call the reduce
    MPI_Reduce(numsThisProc, result, 3, MPI_INT, MPI_SUM, root, MPI_COMM_WORLD);

    if (rank == root) {
        // Check vs. expectation
        if (!faasm::compareArrays<int>(result, expected, 3)) {
            return 1;
        }

        printf("Reduce as expected\n");
    }

    MPI_Finalize();

    return MPI_SUCCESS;
}