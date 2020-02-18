#include <mpi.h>
#include <stdio.h>
#include <faasm/faasm.h>

/**
 * Dumping ground for checking compatibility with various MPI functions
 * @return
 */
FAASM_MAIN_FUNC() {
    MPI_Init(NULL, NULL);

    int rank;
    int worldSize;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &worldSize);

    double mpiTime = MPI_Wtime();
    if(mpiTime < 0) {
        printf("MPI_Wtime messed up (%f)\n", mpiTime);
        return 1;
    }

    int intSize;
    MPI_Type_size(MPI_INT, &intSize);
    if(intSize != 4) {
        printf("MPI_Type_size messed up (%i)\n", intSize);
    }

    MPI_Finalize();

    return 0;
}