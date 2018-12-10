#pragma once

#include "faasm/matrix.h"
#include "faasm/sgd.h"

using namespace faasm;

namespace tests {
    void checkSparseMatrixEquality(const SparseMatrix<double> &a, const SparseMatrix<double> &b);

    double doSgdStep(FaasmMemory *mem, SgdParams &params, int epoch, SparseMatrix<double> &inputs, MatrixXd &outputs);
}
