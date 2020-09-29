#include <faasm/faasm.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __wasm__
#include <clapack.h>
#include <lapacke.h>
#else
// TODO - add native CBLAS build
#endif

/**
 * Checks BLAS support
 *
 * Example copied from http://www.netlib.org/lapack/lapacke.html
 */
FAASM_MAIN_FUNC()
{
    int i, j;
    float err = 0.0;
    
    lapack_int m = 10;
    lapack_int n = 5;
    lapack_int lda = m;

    lapack_complex_float* a = calloc(m * n, sizeof(lapack_complex_float));
    lapack_complex_float* r = calloc(n * n, sizeof(lapack_complex_float));
    lapack_complex_float* tau = calloc(m, sizeof(lapack_complex_float));
    
    lapack_complex_float one = lapack_make_complex_float(1.0, 0.0);
    lapack_complex_float zero = lapack_make_complex_float(0.0, 0.0);
   
    lapack_int info;
    for (int j = 0; j < n; j++) {        
        for (int i = 0; i < m; i++) {            
            a[i + j * m] = lapack_make_complex_float(i + 1, j + 1);
            info = LAPACKE_cgeqrf(LAPACK_COL_MAJOR, m, n, a, lda, tau);
        }
    }

    info = LAPACKE_cungqr(LAPACK_COL_MAJOR, m, n, n, a, lda, tau);
    
    for (int j = 0; j < n; j++) {
        for (int i = 0; i < n; i++) {
            r[i + j * n] = (i == j) ? -one : zero;
        }
    }
    
    // Invoke the CBLAS function
    cblas_cgemm(CblasColMajor,
                CblasConjTrans,
                CblasNoTrans,
                n,
                n,
                m,
                &one,
                a,
                lda,
                a,
                lda,
                &one,
                r,
                n);

    // Calculate error
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            err = MAX(err, cabs(r[i + j * n]));
        }
    }

    printf("CBLAS check error=%e\n", err);

    free(tau);
    free(r);
    free(a);

    return (info);
}
