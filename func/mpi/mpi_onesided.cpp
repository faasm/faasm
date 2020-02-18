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
    int dataSize;
    MPI_Type_size(MPI_INT, &dataSize);
    MPI_Aint memSize = NUM_ELEMENT * dataSize;
    int *sharedData;
    MPI_Alloc_mem(memSize, MPI_INFO_NULL, &sharedData);

    int putData[NUM_ELEMENT];

    int expectedGetData[NUM_ELEMENT];
    int expectedPutData[NUM_ELEMENT];

    for (int i = 0; i < NUM_ELEMENT; i++) {
        // Populate the existing shared mem along with a copy
        sharedData[i] = 10 * rank + i;
        putData[i] = 10 * rank + i;

        // Data we expect to get from one rank and have put into our shared mem by another
        expectedGetData[i] = 10 * getRank + i;
        expectedPutData[i] = 10 * putterRank + i;
    }

    // Create a window on the shared data
    MPI_Win window;
    MPI_Win_create(sharedData, NUM_ELEMENT * dataSize, dataSize, MPI_INFO_NULL, MPI_COMM_WORLD, &window);

    // Get data from another rank's shared memory
    int actual[NUM_ELEMENT];
    MPI_Win_fence(0, window);
    if(rank < 3) {
        MPI_Get(actual, NUM_ELEMENT, MPI_INT, getRank, 0, NUM_ELEMENT, MPI_INT, window);
    }
    MPI_Win_fence(0, window);

    // Check we've got the data we expected
    if(rank < 3 && !faasm::compareIntArrays(actual, expectedGetData, NUM_ELEMENT)) {
        return 1;
    } else {
        printf("Rank %i - MPI_Get as expected\n", rank);
    }

    // Put values to another rank
    MPI_Win_fence(0, window);
    if(rank < 3) {
        MPI_Put(putData, NUM_ELEMENT, MPI_INT, putRank, 0, NUM_ELEMENT, MPI_INT, window);
    }
    MPI_Win_fence(0, window);

    // Check we've had the expected data put in our memory
    if (rank < 3 && !faasm::compareIntArrays(sharedData, expectedPutData, NUM_ELEMENT)) {
        return 1;
    } else {
        printf("Rank %i - MPI_Put as expected\n", rank);
    }

    MPI_Win_free(&window);
    MPI_Finalize();

    return 0;
}
