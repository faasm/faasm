#include "faasm/faasm.h"
#include "faasm/matrix.h"

namespace faasm {
    int exec(FaasmMemory *memory) {
        Eigen::MatrixXd dense = randomDenseMatrix(1, 10);
        Eigen::SparseMatrix<double> sparse = randomSparseMatrix(10, 20, 0.1);

        Eigen::MatrixXd prod = dense * sparse;

        printf("Output elements %f %f %f\n", prod.coeff(0, 0), prod.coeff(0, 1), prod.coeff(0, 2));

        return 0;
    }
}
