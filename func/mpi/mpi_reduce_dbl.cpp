#include <faasm/compare.h>
#include <faasm/faasm.h>
#include <mpi.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char* argv[])
{
    MPI_Init(NULL, NULL);

    int rank;
    int worldSize;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &worldSize);

    // Create an array of three values on this process
    double numsThisProc[3] = { 1.0 * rank, 10.0 * rank, 100.0 * rank };

    int root = 1;
    double* expected = nullptr;
    double* result = nullptr;

    if (rank == root) {
        // Build expectation
        expected = new double[3];
        memset(expected, 0, 3 * sizeof(double));

        for (int r = 0; r < worldSize; r++) {
            expected[0] += r;
            expected[1] += 10.0 * r;
            expected[2] += 100.0 * r;
        }

        result = new double[3];
    }

    // Call the reduce
    MPI_Reduce(
      numsThisProc, result, 3, MPI_DOUBLE, MPI_SUM, root, MPI_COMM_WORLD);

    if (rank == root) {
        // Check vs. expectation
        if (!faasm::compareArrays<double>(result, expected, 3)) {
            return 1;
        }

        printf("Reduce as expected\n");
    }

    delete[] expected;
    delete[] result;

    MPI_Finalize();

    return MPI_SUCCESS;
}
