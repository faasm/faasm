#pragma once

#include "faasm/matrix.h"

using namespace faasm;

namespace tests {

void checkSparseMatrixEquality(const SparseMatrix<double>& a,
                               const SparseMatrix<double>& b);

}
