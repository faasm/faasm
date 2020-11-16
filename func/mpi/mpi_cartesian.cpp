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

    // TODO can I fix the worldSize to 5?
    printf("This is the current world size: %i\n", worldSize);

    // Prepare arguments
    int ndims = 2;
    int dims[2] = { 2, 2 };
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
    if (rank >= 4) {
        expected[0] = expected[1] = MPI_UNDEFINED;
    } else {
        expected[0] = rank / 2;
        expected[1] = rank % 2;
    }
    if (MPI_Cart_get(cart, ndims, dims, periods, coords) != MPI_SUCCESS) {
        printf("MPI_Cart_init failed!\n");
        return 1;
    }

    // Test integrity of results
    // TODO use Faasm compare array
    printf("\nI am rank: %i and these are my coords (%i, %i)\n",
           rank,
           coords[0],
           coords[1]);
    if ((coords[0] != expected[0]) || (coords[1] != expected[1])) {
        printf("Wrong cartesian coordinates.\n");
        return 1;
    }

    printf("MPI_Cart_get succesful check.\n");

    MPI_Finalize();

    return 0;
}
