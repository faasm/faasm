#include "faasm.h"
#include "matrix.h"
#include <stdlib.h>

/**
 * Function to check matrix read/ write
 */

namespace faasm {
    int exec(FaasmMemory *memory) {
        double stackArray[2];
        double stackArrayb[2];
        printf("STACK: %li\n", (long)stackArray);
        printf("STACK: %li\n", (long)stackArrayb);

        const uint8_t *inputPtr = memory->getInput();
        printf("INPUT: %li\n", (long) inputPtr);

        double* heapArray = (double*) malloc (16);
        double* heapArrayb = (double*) malloc (16);
        printf("HEAP: %li\n", (long)heapArray);
        printf("HEAP: %li\n", (long)heapArrayb);

        Matrix<float, 100, 100> mat;
        mat.setOnes();

        printf("ROWS: %ld\n", mat.rows());
        printf("COLS: %ld\n", mat.cols());
        printf("SIZE: %ld\n", sizeof(int));

        printf("MATRIX: \n");
        float *data = mat.data();

        printf("PTR: %p\n", data);
        printf("PTRL: %li\n", (long) data);

        float dat1 = data[0];
        printf("FOO\n");

        mat(3, 4) = dat1 + 3;

        float dat2 = data[1];
        mat(3, 4) += dat2;

        return 0;
    }
}
