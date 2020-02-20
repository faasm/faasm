#include <stdio.h>
#include <mpi.h>
#include <faasm/faasm.h>
#include <faasm/compare.h>

#define NUM_ELEMENT 4

FAASM_MAIN_FUNC() {
    MPI_Init(NULL, NULL);

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int dataSize = sizeof(int);
    int sharedData[NUM_ELEMENT];
    int expectedPutData[NUM_ELEMENT];

    for (int i = 0; i < NUM_ELEMENT; i++) {
        // Populate the existing shared mem along with a copy
        sharedData[i] = 10 * rank + i;
        expectedPutData[i] = 10 * rank + i;
    }

    // Create a window on the shared data
    MPI_Win window;
    MPI_Win_create(sharedData, NUM_ELEMENT * dataSize, 1, MPI_INFO_NULL, MPI_COMM_WORLD, &window);

    // Check we've had the expected data put in our memory
    if(rank < 3) {
        bool putDataEqual = faasm::compareIntArrays(sharedData, expectedPutData, NUM_ELEMENT);

        if (!putDataEqual) {
            printf("Rank %i - stack corrupted by win_create\n", rank);
            return 1;
        } else if (rank < 3) {
            printf("Rank %i - stack not corrupted\n", rank);
        }
    }

    MPI_Win_free(&window);
    MPI_Finalize();

    return 0;
}
