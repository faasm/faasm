#include <stdio.h>
#include <mpi.h>
#include <faasm/faasm.h>
#include <faasm/compare.h>

#define NUM_ELEMENT 4

FAASM_MAIN_FUNC() {
    MPI_Init(NULL, NULL);

    int rank;
    int worldSize;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &worldSize);

    // Get name of host
    char name[MPI_MAX_PROCESSOR_NAME];
    int len;
    MPI_Get_processor_name(name, &len);
    printf("Rank %d running on %s\n", rank, name);

    // Drop out if not in ranks 0-2
    if (rank < 3) {
        MPI_Finalize();
        return 0;
    }

    // Hard-code the other ranks this one will talk to and
    // receive data from
    int getRank;
    int putRank;
    int putterRank;
    if (rank == 0) {
        getRank = 1;
        putRank = 2;
        putterRank = 1;
    } else if (rank == 1) {
        getRank = 2;
        putRank = 0;
        putterRank = 2;
    } else {
        getRank = 0;
        putRank = 1;
        putterRank = 0;
    }

    // Generate some data
    int rankData = MPI_Alloc_mem();
    int expectedGetData[NUM_ELEMENT];
    int expectedPutData[NUM_ELEMENT];
    int putData[NUM_ELEMENT];

    for (int i = 0; i < NUM_ELEMENT; i++) {
        // Data to store in this rank
        rankData[i] = 10 * rank + i;

        // Copy of the rank data
        putData[i] = 10 * rank + i;

        // Data we expect to get/ have put
        expectedGetData[i] = 10 * getRank + i;
        expectedPutData[i] = 10 * putterRank + i;
    }

    // Get size of data
    int dataSize;
    MPI_Type_size(MPI_INT, &dataSize);

    // Create a window and write data to it
    MPI_Win window;
    MPI_Win_create(rankData, NUM_ELEMENT * dataSize, dataSize, MPI_INFO_NULL, MPI_COMM_WORLD, &window);

    // Start epoch
    MPI_Win_fence(0, window);

    // Get values from another rank
    int actual[NUM_ELEMENT];
    MPI_Get(actual, NUM_ELEMENT, MPI_INT, getRank, 0, NUM_ELEMENT, MPI_INT, window);

    // End epoch
    MPI_Win_fence(0, window);

    // Check we've got the data we expected
    if (!faasm::compareIntArrays(actual, expectedGetData, NUM_ELEMENT)) {
        return 1;
    }

    // Start epoch
    MPI_Win_fence(0, window);

    // Put values to neighbour
    MPI_Put(putData, NUM_ELEMENT, MPI_INT, putRank, 0, NUM_ELEMENT, MPI_INT, window);

    // End epoch
    MPI_Win_fence(0, window);

    // Check we've had the expected data put in our window
    if (!faasm::compareIntArrays(rankData, expectedPutData, NUM_ELEMENT)) {
        return 1;
    }

    MPI_Win_free(&window);
    MPI_Finalize();

    return 0;
}
