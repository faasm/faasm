#include "faasm.h"
#include "matrix.h"


/**
 * Function to check matrix read/ write
 */

namespace faasm {
    int exec(FaasmMemory *memory) {
        Matrix<int, 3, 4> mat;
        mat.setOnes();

        printf("ROWS: %ld\n", mat.rows());
        printf("COLS: %ld\n", mat.cols());
        printf("SIZE: %ld\n", sizeof(int));

        printf("MATRIX: \n");
        int *data = mat.data();

        printf("PTR: %p\n", data);
        printf("PTRL: %li\n", (long) data);

        int dat1 = data[0];
        printf("FOO\n");
        printf("DAT 1: %d\n", dat1);

        int dat2 = data[1];
        printf("DAT 2: %d\n", dat2);

        return 0;
    }
}
