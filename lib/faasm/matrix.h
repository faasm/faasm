#ifndef FAASM_MATRIX_H
#define FAASM_MATRIX_H

#include "faasm/memory.h"
#include "faasm/random.h"

#include <eigen3/Eigen/Core>
#include <eigen3/Eigen/SparseCore>

using namespace Eigen;

namespace faasm {

    void writeSparseMatrixToState(FaasmMemory *memory, const SparseMatrix<double> &mat, const char* key);

    SparseMatrix<double> readSparseMatrixFromState(FaasmMemory *memory, const char* key);

    MatrixXd randomDenseMatrix(int rows, int cols);

    SparseMatrix<double> randomSparseMatrix(int rows, int cols, double threshold);

    uint8_t *matrixToBytes(const MatrixXd &mat);

    MatrixXd bytesToMatrix(uint8_t *byteArray, long rows, long columns);

    void writeMatrixToState(FaasmMemory *memory, const char *key, const MatrixXd &matrix);

    MatrixXd readMatrixFromState(FaasmMemory *memory, const char *key, long rows, long cols);

    long matrixByteIndex(long row, long col, long nRows);

    void writeMatrixToStateElement(FaasmMemory *memory, const char *key, const MatrixXd &matrix, long row, long col);

    MatrixXd readMatrixColumnsFromState(FaasmMemory *memory, const char *key, long colStart, long colEnd, long nRows);

    SparseMatrix<double> readSparseMatrixColumnsFromState(FaasmMemory *memory, const char *key, long colStart,
            long colEnd);

    void shuffleMatrixColumns(MatrixXd &matrix);

    void shufflePairedMatrixColumns(MatrixXd &a, MatrixXd &b);

    void printMatrix(MatrixXd &mat);

    double calculateSquaredError(const MatrixXd &a, const MatrixXd &b);

    double calculateRootMeanSquaredError(const MatrixXd &a, const MatrixXd &b);
}

#endif
