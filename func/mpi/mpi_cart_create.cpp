#include <cmath>
#include <faasm/compare.h>
#include <faasm/faasm.h>
#include <mpi.h>
#include <stdio.h>

int main(int argc, char* argv[])
{
    MPI_Init(NULL, NULL);

    int rank;
    int worldSize;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &worldSize);

    // Prepare arguments
    int ndims = 2;
    int sideLength = static_cast<int>(std::floor(std::sqrt(worldSize)));
    int nprocs = sideLength * sideLength;
    int dims[2] = { sideLength, sideLength };
    int periods[2] = { 0, 0 };
    int reorder = 0;
    MPI_Comm cart1, cart2;

    // Create two different communicators
    if (MPI_Cart_create(
          MPI_COMM_WORLD, ndims, dims, periods, reorder, &cart1) !=
        MPI_SUCCESS) {
        printf("MPI_Cart_create failed!\n");
        return 1;
    }
    if (MPI_Cart_create(
          MPI_COMM_WORLD, ndims, dims, periods, reorder, &cart2) !=
        MPI_SUCCESS) {
        printf("MPI_Cart_create failed!\n");
        return 1;
    }

    // Check that they have been allocated at different addresses. This is to
    // prevent situations in which memory is not properly allocated and we
    // end up creating an object at the base of the wasm memory array.
    if (&cart1 == &cart2) {
        printf("Both communicators allocated at the same address.\n");
        return 1;
    }
    printf("MPI_Cart_create correctly allocated.\n");

    MPI_Finalize();

    return 0;
}
