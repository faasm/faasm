#include <faasm/faasm.h>
#include <mpi.h>
#include <stdio.h>

FAASM_MAIN_FUNC()
{
    MPI_Init(NULL, NULL);

    int rank;
    int worldSize;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &worldSize);

    if (worldSize < 3) {
        printf("Need world size of 3 or more\n");
        return 1;
    }

    if (rank == 0) {
        // Send messages out
        int out[3] = { 111, 222, 333 };
        MPI_Send(&out[0], 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
        MPI_Send(&out[1], 1, MPI_INT, 2, 0, MPI_COMM_WORLD);
        MPI_Send(&out[2], 1, MPI_INT, 3, 0, MPI_COMM_WORLD);

        // Get responses out of order
        int in[3] = { 0, 0, 0 };
        MPI_Recv(&in[2], 1, MPI_INT, 3, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&in[0], 1, MPI_INT, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&in[1], 1, MPI_INT, 2, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        // Check values
        if (out[0] != in[0] || out[1] != in[1] || out[2] != in[2]) {
            printf("Responses not as expected (got [%i, %i, %i], expected [%i, "
                   "%i, %i]\n",
                   in[0],
                   in[1],
                   in[2],
                   out[0],
                   out[1],
                   out[2]);
            return 1;
        } else {
            printf("MPI order check successful\n");
        }

    } else if (rank > 0 && rank <= 3) {
        // Echo message back to master
        int receivedNumber = 0;
        MPI_Recv(
          &receivedNumber, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Send(&receivedNumber, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
    }

    MPI_Finalize();

    return 0;
}
