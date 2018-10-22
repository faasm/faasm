#include <eigen3/Eigen/Dense>

#include "faasm.h"
#include "matrix.h"

namespace faasm {
    int exec(FaasmMemory *memory) {
        Eigen::MatrixXd m(2, 2);
        printf("Matrix initialised\n");

        m(0, 0) = 3;
        m(1, 0) = 2.5;
        m(0, 1) = -1;
        m(1, 1) = m(1, 0) + m(0, 1);
        printf("Matrix set up\n");

        return 0;
    }
}