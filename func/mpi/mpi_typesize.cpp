#include <stdio.h>
#include <mpi.h>
#include <faasm/faasm.h>

bool checkTypeSize(MPI_Datatype dt, int expected, const char* name) {
    int actual;
    MPI_Type_size(dt, &actual);
    if(actual != expected) {
        printf("MPI %s size not as expected (got %i, expected %i)\n", name, actual, expected);
        return false;
    } else {
        return true;
    }
}

FAASM_MAIN_FUNC() {
    MPI_Init(NULL, NULL);

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if(rank == 0) {
        if(!checkTypeSize(MPI_INT, sizeof(int), "int")) return 1;
        if(!checkTypeSize(MPI_LONG, sizeof(long), "long")) return 1;
        if(!checkTypeSize(MPI_FLOAT, sizeof(float), "float")) return 1;

        printf("MPI type sizes as expected\n");
    }

    MPI_Finalize();

    return 0;
}
