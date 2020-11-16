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
    MPI_Comm cart;

    // Test MPI_Cart_create
    if (MPI_Cart_create(MPI_COMM_WORLD, ndims, dims, periods, reorder, &cart) !=
        MPI_SUCCESS) {
        printf("MPI_Cart_init failed!\n");
        return 1;
    }
    printf("MPI_Cart_create succesful check.\n");

    // Test MPI_Cart_get
    int coords[2];
    int expected[2];
    if (rank >= nprocs) {
        expected[0] = expected[1] = MPI_UNDEFINED;
    } else {
        expected[0] = rank / sideLength;
        expected[1] = rank % sideLength;
    }
    if (MPI_Cart_get(cart, ndims, dims, periods, coords) != MPI_SUCCESS) {
        printf("MPI_Cart_get failed!\n");
        return 1;
    }

    // Test integrity of results
    if (!faasm::compareArrays<int>(coords, expected, 2)) {
        printf("Wrong cartesian coordinates.\n");
        return 1;
    }
    printf("MPI_Cart_get succesful check.\n");

    printf("still here\n");
    // Now check that the original rank can be retrieved.
    if (rank < nprocs) {
        int nRank;
        if (MPI_Cart_rank(cart, coords, &nRank) != MPI_SUCCESS) {
            printf("MPI_Cart_rank failed!\n");
            return 1;
        }
        if (rank != nRank) {
            printf(
              "Wrong rank from cartesian coordinates. Expected: %i Got: %i",
              rank,
              nRank);
            return 1;
        }
        printf("MPI_Cart_rank succesful check.\n");
    }

    printf("MPI Cartesian Topology Checks Succesful.\n");
    MPI_Finalize();

    return 0;
}
