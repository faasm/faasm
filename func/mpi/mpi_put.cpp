#include <faasm/compare.h>
#include <faasm/faasm.h>
#include <mpi.h>
#include <stdio.h>

#define NUM_ELEMENT 4

int main(int argc, char* argv[])
{
    MPI_Init(NULL, NULL);

    int rank;
    int worldSize;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &worldSize);

    size_t elemSize = sizeof(int);

    int* sharedData;
    MPI_Alloc_mem(elemSize * NUM_ELEMENT, MPI_INFO_NULL, &sharedData);

    MPI_Win window;
    MPI_Win_create(sharedData,
                   NUM_ELEMENT * elemSize,
                   1,
                   MPI_INFO_NULL,
                   MPI_COMM_WORLD,
                   &window);
    MPI_Win_fence(0, window);

    // Put values from rank 1 to rank 0
    int putData[NUM_ELEMENT];
    if (rank == 1) {
        for (int i = 0; i < NUM_ELEMENT; i++) {
            putData[i] = 10 + i;
        }

        MPI_Put(
          putData, NUM_ELEMENT, MPI_INT, 0, 0, NUM_ELEMENT, MPI_INT, window);
    }

    MPI_Win_fence(0, window);

    // Check expectation on rank 1
    if (rank == 0) {
        int expected[NUM_ELEMENT];
        for (int i = 0; i < NUM_ELEMENT; i++) {
            expected[i] = 10 + i;
        }

        bool asExpected =
          faasm::compareArrays<int>(sharedData, expected, NUM_ELEMENT);
        if (!asExpected) {
            return 1;
        }
        printf("Rank %i - MPI_Put as expected\n", rank);
    }

    MPI_Win_free(&window);
    MPI_Finalize();

    return 0;
}
