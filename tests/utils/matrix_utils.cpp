#include <catch2/catch.hpp>

#include "utils.h"

using namespace faasm;

namespace tests {

    void checkSparseMatrixEquality(const SparseMatrix<double> &a, const SparseMatrix<double> &b) {
        // As we can't use a simple equality operator, we need to iterate through both
        // matrices and check the contents in detail

        REQUIRE(a.nonZeros() == b.nonZeros());
        REQUIRE(a.cols() == b.cols());
        REQUIRE(a.rows() == b.rows());

        std::vector<int> rowsA;
        std::vector<int> colsA;
        std::vector<double> valuesA;

        for (int k = 0; k < a.outerSize(); ++k) {
            for (SparseMatrix<double>::InnerIterator it(a, k); it; ++it) {
                valuesA.push_back(it.value());
                rowsA.push_back(it.row());
                colsA.push_back(it.col());
            }
        }

        std::vector<int> rowsB;
        std::vector<int> colsB;
        std::vector<double> valuesB;

        for (int k = 0; k < b.outerSize(); ++k) {
            for (SparseMatrix<double>::InnerIterator it(b, k); it; ++it) {
                valuesB.push_back(it.value());
                rowsB.push_back(it.row());
                colsB.push_back(it.col());
            }
        }

        REQUIRE(rowsA == rowsB);
        REQUIRE(colsA == colsB);
        REQUIRE(valuesA == valuesB);
    }
}
