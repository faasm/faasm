#ifndef FAASM_MATRIX_H
#define FAASM_MATRIX_H

#include "faasm/memory.h"
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

        void writeToState(FaasmMemory *memory, const char *key, bool async);

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
    Map<const SparseMatrix<double>> readSparseMatrixFromState(FaasmMemory *memory, const char *key, bool async);

    void writeSparseMatrixToState(FaasmMemory *memory, const char *key, const SparseMatrix<double> &mat,
                                  bool async);

    Map<const MatrixXd> readMatrixFromState(FaasmMemory *memory, const char *key, long rows, long cols, bool async);

    void readMatrixFromState(FaasmMemory *memory, const char *key, double *buffer, long rows, long cols,
                                 bool async);

    void writeMatrixToState(FaasmMemory *memory, const char *key, const MatrixXd &matrix, bool async);

    void writeMatrixToStateElement(FaasmMemory *memory, const char *key, const MatrixXd &matrix, long row, long col,
                                   bool async);

    Map<const MatrixXd> readMatrixColumnsFromState(FaasmMemory *memory, const char *key, long totalCols, long colStart,
                                        long colEnd, long nRows, bool async);

    Map<const SparseMatrix<double>> readSparseMatrixColumnsFromState(FaasmMemory *memory, const char *key, long colStart,
                                                          long colEnd, bool async);

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
