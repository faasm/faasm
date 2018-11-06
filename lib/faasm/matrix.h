#ifndef FAASM_MATRIX_H
#define FAASM_MATRIX_H

#include "faasm/memory.h"
#include "faasm/random.h"

#include <random>
#include <eigen3/Eigen/Core>
#include <eigen3/Eigen/SparseCore>

using namespace Eigen;

namespace faasm {
    MatrixXd randomDenseMatrix(int rows, int cols);

    MatrixXd randomSparseMatrix(int rows, int cols);

    uint8_t *matrixToBytes(const MatrixXd &mat);

    MatrixXd bytesToMatrix(uint8_t *byteArray, long rows, long columns);

    void writeMatrixState(FaasmMemory *memory, const char *key, const MatrixXd &matrix);

    MatrixXd readMatrixFromState(FaasmMemory *memory, const char *key, long rows, long cols);

    long matrixByteIndex(long row, long col, long nRows);

    void writeMatrixStateElement(FaasmMemory *memory, const char *key, const MatrixXd &matrix, long row, long col);

    MatrixXd readMatrixColumnsFromState(FaasmMemory *memory, const char *key, long colStart, long colEnd, long nRows);

    void shuffleMatrixColumns(MatrixXd &matrix);

    void printMatrix(MatrixXd &mat);

    double calculateSquaredError(const MatrixXd &a, const MatrixXd &b);

    double calculateMse(const MatrixXd &a, const MatrixXd &b);
}

#endif
