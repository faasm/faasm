#include <faasm/compare.h>
#include <faasm/faasm.h>
#include <mpi.h>
#include <stdio.h>

#define NUM_ELEMENT 4

int main(int argc, char* argv[])
{
    MPI_Init(NULL, NULL);

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // Get name of host
    char* name = new char[MPI_MAX_PROCESSOR_NAME];
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
    int dataSize = sizeof(int);
    MPI_Type_size(MPI_INT, &dataSize);
    MPI_Aint memSize = NUM_ELEMENT * dataSize;
    int* sharedData;
    MPI_Alloc_mem(memSize, MPI_INFO_NULL, &sharedData);

    int* putData = new int[NUM_ELEMENT];
    int* expectedGetData = new int[NUM_ELEMENT];
    int* expectedPutData = new int[NUM_ELEMENT];

    for (int i = 0; i < NUM_ELEMENT; i++) {
        // Populate the existing shared mem along with a copy
        sharedData[i] = 10 * rank + i;
        putData[i] = 10 * rank + i;

        // Data we expect to get from one rank and have put into our shared mem
        // by another
        expectedGetData[i] = 10 * getRank + i;
        expectedPutData[i] = 10 * putterRank + i;
    }

    // Create a window on the shared data
    MPI_Win window;
    MPI_Win_create(sharedData,
                   NUM_ELEMENT * dataSize,
                   1,
                   MPI_INFO_NULL,
                   MPI_COMM_WORLD,
                   &window);
    MPI_Win_fence(0, window);

    // Get data from another rank's shared memory
    int actual[NUM_ELEMENT];
    if (rank < 3) {
        MPI_Get(actual,
                NUM_ELEMENT,
                MPI_INT,
                getRank,
                0,
                NUM_ELEMENT,
                MPI_INT,
                window);
    }
    MPI_Win_fence(0, window);

    // Check we've got the data we expected
    if (rank < 3 &&
        !faasm::compareArrays<int>(actual, expectedGetData, NUM_ELEMENT)) {
        return 1;
    } else if (rank < 3) {
        printf("Rank %i - MPI_Get as expected\n", rank);
    }

    // Put values to another rank
    if (rank < 3) {
        MPI_Put(putData,
                NUM_ELEMENT,
                MPI_INT,
                putRank,
                0,
                NUM_ELEMENT,
                MPI_INT,
                window);
    }
    MPI_Win_fence(0, window);

    // Check we've had the expected data put in our memory
    if (rank < 3 &&
        !faasm::compareArrays<int>(sharedData, expectedPutData, NUM_ELEMENT)) {
        return 1;
    } else if (rank < 3) {
        printf("Rank %i - MPI_Put as expected\n", rank);
    }

    delete[] expectedGetData;
    delete[] expectedPutData;
    delete[] name;
    delete[] putData;

    MPI_Win_free(&window);
    MPI_Finalize();

    return 0;
}
