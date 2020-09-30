#include <stdio.h>
#include <stdlib.h>

#ifdef __wasm__
#include <blaswrap.h>
#include <f2c.h>
#include <clapack.h>
#include <math.h>
#else
// TODO - add native CBLAS build
#endif

#define NDIM 4

/**
 * Checks BLAS support
 *
 * Example copied from http://theochem.mercer.edu/clapack/dgesv_example.c
 */
int main()
{
    int i, j, info2;
    int N, NRHS, LDA, LDB;
    double* A;
    double* B;
    static int IPIV[NDIM], INFO;

    A = (double*)malloc(NDIM * NDIM * sizeof(double));
    B = (double*)malloc(NDIM * sizeof(double));

    N = NDIM;
    NRHS = 1;
    LDA = NDIM;
    LDB = NDIM;

    A[0] = 1.0;
    A[4] = -1.0;
    A[8] = 2.0;
    A[12] = -1.0;
    A[1] = 2.0;
    A[5] = -2.0;
    A[9] = 3.0;
    A[13] = -3.0;
    A[2] = 1.0;
    A[6] = 1.0;
    A[10] = 1.0;
    A[14] = 0.0;
    A[3] = 1.0;
    A[7] = -1.0;
    A[11] = 4.0;
    A[15] = 3.0;

    // Print matrix A
    for (i = 0; i < N; i++) {
        printf("\n");
        for (j = 0; j < N; j++) {
            printf("%2.1f,", A[i + N * j]);
        }
    }
    printf("\n");

    // Set up vector B
    B[0] = -8.0;
    B[1] = -20.0;
    B[2] = -2.0;
    B[3] = 4.0;

    dgesv_(&N, &NRHS, A, &LDA, &IPIV, B, &LDB, &INFO);

    printf("Info %d \n", INFO);
    for (i = 0; i < N; i++) {
        printf("   %f \n", B[i]);
    }

    return 0;
}

