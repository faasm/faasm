/*
Copyright (c) 2013, Intel Corporation

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

* Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above
      copyright notice, this list of conditions and the following
      disclaimer in the documentation and/or other materials provided
      with the distribution.
* Neither the name of Intel Corporation nor the names of its
      contributors may be used to endorse or promote products
      derived from this software without specific prior written
      permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
*/

/*******************************************************************

NAME:    transpose

PURPOSE: This program tests the efficiency with which a square matrix
         can be transposed and stored in another matrix. The matrices
         are distributed identically.

USAGE:   Program input is three command line arguments that give the
         matrix order, the number of times to repeat the operation
         (iterations), and the number of threads to use:

         transpose <# threads> <# iterations> <matrix_size> [tile size]

         An optional parameter specifies the tile size used to divide the
         individual matrix blocks for improved cache and TLB performance.

         The output consists of diagnostics to make sure the
         transpose worked and timing statistics.


FUNCTIONS CALLED:

         Other than OpenMP or standard C functions, the following
         functions are used in this program:

         wtime()          portable wall-timer interface.
         bail_out()
         test_results()   Verify that the transpose worked

HISTORY: Written by Tim Mattson, April 1999.
         Updated by Rob Van der Wijngaart, December 2005.

*******************************************************************/

#include "./intel_includes/par-res-kern_general.h"
#include "./intel_includes/par-res-kern_omp.h"
#include <faasm/faasm.h>

#define A(i,j)    A[i+order*(j)]
#define B(i,j)    B[i+order*(j)]
static double test_results (size_t , double*, int);

// int main(int argc, char ** argv) {
FAASM_MAIN_FUNC() {

    size_t order;         /* order of a the matrix                           */
    size_t i, j, it, jt;  /* matrix/tile indices                             */
    int    Tile_order=32; /* default tile size for tiling of local transpose */
    int    iterations;    /* number of times to do the transpose             */
    int    iter;          /* dummy                                           */
    int    tiling;        /* boolean: true if tiling is used                 */
    double bytes;         /* combined size of matrices                       */
    double * RESTRICT A;  /* buffer to hold original matrix                  */
    double * RESTRICT B;  /* buffer to hold transposed matrix                */
    double abserr;        /* absolute error                                  */
    double epsilon=1.e-8; /* error tolerance                                 */
    double transpose_time,/* timing parameters                               */
            avgtime;
    int    nthread_input,
            nthread;
    int    num_error=0;     /* flag that signals that requested and
                             obtained numbers of threads are the same      */

    /*********************************************************************
    ** read and test input parameters
    *********************************************************************/

    printf("Parallel Research Kernels version %s\n", PRKVERSION);
    printf("OpenMP Matrix transpose: B = A^T\n");

    nthread_input = 4;
    iterations    = 300;
    order         = 1000;
    /* Faasm - Hardcode the default
    if (argc != 4 && argc != 5){
        printf("Usage: %s <# threads> <# iterations> <matrix order> [tile size]\n",
               *argv);
        // Faasm - Use above defaults
    }
    else {
        nthread_input = atoi(*++argv);
        iterations    = atoi(*++argv);
        order         = atoi(*++argv);
    }
    */
    // Faasm - compiler warning uninitialised:
    j = 0;
    tiling = 0;

    /* Take number of threads to request from command line */

    if ((nthread_input < 1) || (nthread_input > MAX_THREADS)) {
        printf("ERROR: Invalid number of threads: %d\n", nthread_input);
        exit(EXIT_FAILURE);
    }

    omp_set_num_threads(nthread_input);

    if (iterations < 1){
        printf("ERROR: iterations must be >= 1 : %d \n",iterations);
        exit(EXIT_FAILURE);
    }

    if (order <= 0){
        printf("ERROR: Matrix Order must be greater than 0 : %zu \n", order);
        exit(EXIT_FAILURE);
    }

/*
    if (argc == 5) Tile_order = atoi(*++argv);
    // a non-positive tile size means no tiling of the local transpose
    tiling = (Tile_order > 0) && ((size_t)Tile_order < order);
    if (!tiling) Tile_order = order;
*/

    /*********************************************************************
    ** Allocate space for the input and transpose matrix
    *********************************************************************/

    A   = (double *)prk_malloc(order*order*sizeof(double));
    if (A == NULL){
        printf(" ERROR: cannot allocate space for input matrix: %ld\n",
               order*order*sizeof(double));
        exit(EXIT_FAILURE);
    }
    B  = (double *)prk_malloc(order*order*sizeof(double));
    if (B == NULL){
        printf(" ERROR: cannot allocate space for output matrix: %ld\n",
               order*order*sizeof(double));
        exit(EXIT_FAILURE);
    }

    bytes = 2.0 * sizeof(double) * order * order;

#pragma omp parallel default(none) shared(nthread, num_error, tiling, order, iterations, nthread_input, Tile_order) private (i, j, it, jt, iter)
    {

#pragma omp master
        {
            nthread = omp_get_num_threads();
            if (nthread != nthread_input) {
                num_error = 1;
                printf("ERROR: number of requested threads %d does not equal ",
                       nthread_input);
                printf("number of spawned threads %d\n", nthread);
            }
            else {
                printf("Number of threads     = %i;\n",nthread_input);
                printf("Matrix order          = %ld\n", order);
                printf("Number of iterations  = %d\n", iterations);
                if (tiling) {
                    printf("Tile size             = %d\n", Tile_order);
#if COLLAPSE
                    printf("Loop collapse         = on\n");
#else
                    printf("Loop collapse         = off\n");
#endif
                }
                else
                    printf("Untiled\n");
            }
        }
        bail_out(num_error);

        /*  Fill the original matrix, set transpose to known garbage value. */

        if (tiling) {
#if COLLAPSE
#pragma omp for collapse(2)
#else
#pragma omp for
#endif
            for (j=0; j<order; j+=Tile_order)
                for (i=0; i<order; i+=Tile_order)
                    for (jt=j; jt<MIN(order,j+Tile_order);jt++)
                        for (it=i; it<MIN(order,i+Tile_order); it++){
                            A(it,jt) = (double) (order*jt + it);
                            B(it,jt) = 0.0;
                        }
        }
        else {
#pragma omp for
            for (j=0;j<order;j++)
                for (i=0;i<order; i++) {
                    A(i,j) = (double) (order*j + i);
                    B(i,j) = 0.0;
                }
        }

        for (iter = 0; iter<=iterations; iter++){

            /* start timer after a warmup iteration                                        */
            if (iter == 1) {
#pragma omp barrier
#pragma omp master
                {
                    transpose_time = wtime();
                }
            }

            /* Transpose the  matrix                                                       */
            if (!tiling) {
#pragma omp for
                for (i=0;i<order; i++)
                    for (j=0;j<order;j++) {
                        B(j,i) += A(i,j);
                        A(i,j) += 1.0;
                    }
            }
            else {
#if COLLAPSE
#pragma omp for collapse(2)
#else
#pragma omp for
#endif
                for (i=0; i<order; i+=Tile_order)
                    for (j=0; j<order; j+=Tile_order)
                        for (it=i; it<MIN(order,i+Tile_order); it++)
                            for (jt=j; jt<MIN(order,j+Tile_order);jt++) {
                                B(jt,it) += A(it,jt);
                                A(it,jt) += 1.0;
                            }
            }

        }  /* end of iter loop  */

#pragma omp barrier
#pragma omp master
        {
            transpose_time = wtime() - transpose_time;
        }

    } /* end of OpenMP parallel region */

    abserr =  test_results (order, B, iterations);

    prk_free(B);
    prk_free(A);

    /*********************************************************************
    ** Analyze and output results.
    *********************************************************************/

    if (abserr < epsilon) {
        printf("Solution validates\n");
        avgtime = transpose_time/iterations;
        printf("Rate (MB/s): %lf Avg time (s): %lf\n",
               1.0E-06 * bytes/avgtime, avgtime);
#if VERBOSE
        printf("Squared errors: %f \n", abserr);
#endif
        exit(EXIT_SUCCESS);
    }
    else {
        printf("ERROR: Aggregate squared error %lf exceeds threshold %e\n",
               abserr, epsilon);
        exit(EXIT_FAILURE);
    }

}  /* end of main */



/* function that computes the error committed during the transposition */

double test_results (size_t order, double *B, int iterations) {

    double abserr=0.0;
    size_t i, j = 0; // Faasm - Add = 0 for compiler warnings

    double addit = ((double)(iterations+1) * (double) (iterations))/2.0;
#pragma omp parallel for reduction(+:abserr)
    for (j=0;j<order;j++) {
        for (i=0;i<order; i++) {
            abserr += ABS(B(i,j) - ((i*order + j)*(iterations+1L)+addit));
        }
    }

#if VERBOSE
    #pragma omp master
  {
  printf(" Squared sum of differences: %f\n",abserr);
  }
#endif
    return abserr;
}
