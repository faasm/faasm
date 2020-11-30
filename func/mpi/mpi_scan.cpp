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

    int count = 3;
    int* expected = nullptr;
    int* input = nullptr;
    int* result = nullptr;

    expected = new int[count];
    memset(expected, 0, count * sizeof(int));
    input = new int[count];
    memset(input, 0, count * sizeof(int));
    result = new int[count];
    memset(result, 0, count * sizeof(int));

    // Prepare input and expected data
    for (int i = 0; i < count; i++) {
        input[i] = rank * 10 + i;
        for (int r = 0; r <= rank; r++) {
            expected[i] += r * 10 + i;
        }
    }

    // Call the scan operation
    MPI_Scan(input, result, count, MPI_INT, MPI_SUM, MPI_COMM_WORLD);

    // Check vs. expectation
    if (!faasm::compareArrays<int>(result, expected, count)) {
        return 1;
    }
    printf("MPI_Scan not in-place as expected.\n");

    MPI_Barrier(MPI_COMM_WORLD);

    // Check operation in place
    MPI_Scan(input, input, count, MPI_INT, MPI_SUM, MPI_COMM_WORLD);

    // Check vs. expectation
    if (!faasm::compareArrays<int>(input, expected, count)) {
        return 1;
    }
    printf("MPI_Scan in-place as expected.\n");

    MPI_Finalize();

    return MPI_SUCCESS;
}
