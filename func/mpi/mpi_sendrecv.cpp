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

    // Send and receive messages in a ring
    // Sendrecv prevents worrying about possible deadlocks
    int right = (rank + 1) % worldSize;
    int maxRank = worldSize - 1;
    int left = rank > 0 ? rank - 1 : maxRank;

    // Receive from the left and send to the right
    int sendValue = rank;
    int recvValue = -1;
    printf(
      "Rank: %i. Receiving from: %i - Sending to: %i\n", rank, left, right);
    MPI_Sendrecv(&sendValue,
                 1,
                 MPI_INT,
                 right,
                 0,
                 &recvValue,
                 1,
                 MPI_INT,
                 rank,
                 0,
                 MPI_COMM_WORLD,
                 nullptr);

    // Check the received value is as expected
    if (recvValue != left) {
        printf("Rank %i - sendrecv not working properly (got %i expected %i)\n",
               rank,
               recvValue,
               left);
        return 1;
    }
    printf("Rank %i - sendrecv working properly\n", rank);

    MPI_Finalize();

    return 0;
}
