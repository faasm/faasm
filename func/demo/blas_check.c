#include <stdio.h>
#include <stdlib.h>

#ifdef __wasm__
#include <blaswrap.h>
#include <f2c.h>
#include <clapack.h>
#else
// TODO - add native CBLAS build
#endif

#include <math.h>

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
    double* expected;
    double diff;
    static int IPIV[NDIM], INFO;

    A = (double*)malloc(NDIM * NDIM * sizeof(double));
    B = (double*)malloc(NDIM * sizeof(double));
    expected = (double*)malloc(NDIM * sizeof(double));

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

    // Set up vector B
    B[0] = -8.0;
    B[1] = -20.0;
    B[2] = -2.0;
    B[3] = 4.0;

    // Set up expectation
    expected[0] = -7.0;
    expected[1] = 3.0;
    expected[2] = 2.0;
    expected[3] = 2.0;

    dgesv_(&N, &NRHS, A, &LDA, &IPIV, B, &LDB, &INFO);

    // Hacky equality check here as we know they're ints
    int success = 1;
    for(i = 0; i < N; i++) {
        diff = fabs(B[i] - expected[i]);               
        if(diff > 0.00000001) {
            success = 0;
        }
    }

    // Error if we don't get what we expect
    if(success == 0) {
        printf("BLAS check not producing expected: \n");
        for(i = 0; i < N; i++) {
            printf("a[%i] = %2.f   e[%i] = %.2f\n", i, B[i], i, expected[i]);
        }

        return 1;
    } else {
        return 0;
    }
}

