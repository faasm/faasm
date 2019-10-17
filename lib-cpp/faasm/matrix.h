#ifndef FAASM_MATRIX_H
#define FAASM_MATRIX_H

#include "faasm/core.h"
#include "faasm/random.h"

#include <eigen3/Eigen/Core>
#include <eigen3/Eigen/SparseCore>

using namespace Eigen;

namespace faasm {

    // Use ints here as longs are equivalent in wasm world
    struct SparseSizes {
        int cols;
        int rows;
        int nNonZeros;

        int valuesLen;
        int innerLen;
        int outerLen;
        int nonZeroLen;
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

        void writeToState(const char *key, bool push);

        static Map<const SparseMatrix<double>> readFromBytes(const SparseSizes &sizes,
                                                             uint8_t *outerBytes,
                                                             uint8_t *innerBytes,
                                                             uint8_t *valuesBytes);

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

    /**
     * State
     */
    Map<const SparseMatrix<double>> readSparseMatrixFromState(const char *key, bool pull);

    void writeSparseMatrixToState(const char *key, const SparseMatrix<double> &mat, bool push);

    Map<const MatrixXd> readMatrixFromState(const char *key, long rows, long cols, bool pull);

    void readMatrixFromState(const char *key, double *buffer, long rows, long cols, bool pull);

    void writeMatrixToState(const char *key, const MatrixXd &matrix, bool push);

    void writeMatrixToStateElement(const char *key, const MatrixXd &matrix, long row, long col, bool push);

    Map<const MatrixXd> readMatrixColumnsFromState(const char *key, long totalCols, long colStart,
                                                   long colEnd, long nRows, bool pull);

    Map<const SparseMatrix<double>> readSparseMatrixColumnsFromState(const char *key, long colStart, long colEnd,
            bool pull);

    /**
     * Manipulation
     */
    MatrixXd randomDenseMatrix(int rows, int cols);

    MatrixXd zeroMatrix(int rows, int cols);

    SparseMatrix<double> randomSparseMatrix(int rows, int cols, double threshold);

    long matrixByteIndex(long row, long col, long nRows);

    void shuffleMatrixColumns(MatrixXd &matrix);

    void shufflePairedMatrixColumns(MatrixXd &a, MatrixXd &b);

    double calculateSquaredError(const MatrixXd &prediction, const MatrixXd &b);

    double calculateHingeError(const MatrixXd &a, const MatrixXd &b);

    double calculateRootMeanSquaredError(const MatrixXd &a, const MatrixXd &b);
}

#endif
