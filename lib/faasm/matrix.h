#ifndef FAASM_MATRIX_H
#define FAASM_MATRIX_H

#include "faasm/memory.h"
#include "faasm/random.h"

#include <eigen3/Eigen/Core>
#include <eigen3/Eigen/SparseCore>

using namespace Eigen;

namespace faasm {

    struct SparseSizes {
        long cols;
        long rows;
        long nNonZeros;

        size_t valuesLen;
        size_t innerLen;
        size_t outerLen;
    };

    struct SparseKeys {
        char *valueKey;
        char *innerKey;
        char *outerKey;
        char *sizeKey;
        char *nonZeroKey;

        ~SparseKeys() {
            delete[] valueKey;
            delete[] innerKey;
            delete[] outerKey;
            delete[] sizeKey;
            delete[] nonZeroKey;
        }
    };

    SparseKeys getSparseKeys(const char *key);

    class SparseMatrixSerialiser {
    public:
        explicit SparseMatrixSerialiser(const SparseMatrix<double> &matIn);

        void writeToState(FaasmMemory *memory, const char *key);

        static SparseMatrix<double> readFromBytes(const SparseSizes &sizes,
                                                  uint8_t * outerBytes,
                                                  uint8_t * innerBytes,
                                                  uint8_t * valuesBytes);

        ~SparseMatrixSerialiser();

        const SparseMatrix<double> &mat;

        const uint8_t *valueBytes;
        size_t nValueBytes;

        const uint8_t *innerBytes;
        size_t nInnerBytes;

        const uint8_t *outerBytes;
        size_t nOuterBytes;

        const uint8_t *nonZeroBytes;
        size_t nNonZeroBytes;

        const uint8_t *sizeBytes;
        size_t nSizeBytes;

    private:
        int *nonZeroCounts;
        SparseSizes sizes{};
    };

    void writeSparseMatrixToState(FaasmMemory *memory, const char *key, const SparseMatrix<double> &mat);

    SparseMatrix<double> readSparseMatrixFromState(FaasmMemory *memory, const char *key);

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

    double calculateSquaredError(const MatrixXd &a, const MatrixXd &b);

    double calculateRootMeanSquaredError(const MatrixXd &a, const MatrixXd &b);
}

#endif
