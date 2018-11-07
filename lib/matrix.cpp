#include "faasm/matrix.h"

#include <iostream>

using namespace Eigen;

namespace faasm {

    MatrixXd randomDenseMatrix(int rows, int cols) {
        MatrixXd mat = MatrixXd::Random(rows, cols);

        return mat;
    }

    SparseMatrix<double> randomSparseMatrix(int rows, int cols) {
        // Random distribution
        std::default_random_engine gen;
        std::uniform_real_distribution<double> dist(0.0, 1.0);

        // Create a list of triplets
        std::vector<Triplet<double>> triplets;

        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                // Get random value
                auto v_ij = dist(gen);

                // If below threshold, add the triplet to the list
                if (v_ij < 0.1) {
                    const Triplet<double, int> &triplet = Triplet<double>(i, j, v_ij);
                    triplets.push_back(triplet);
                }
            }
        }

        // Create a sparse matrix from the list of triplets
        SparseMatrix<double> mat(rows, cols);
        mat.setFromTriplets(triplets.begin(), triplets.end());

        return mat;
    }

    /**
     * Serialises a matrix to a byte array
     */
    uint8_t *matrixToBytes(const MatrixXd &mat) {
        long rows = mat.rows();
        long cols = mat.cols();

        long nDoubles = rows * cols;
        auto doubleArray = new double[nDoubles];
        Map<MatrixXd>(&doubleArray[0], rows, cols) = mat;

        auto byteArray = reinterpret_cast<uint8_t *>(&doubleArray[0]);

        return byteArray;
    }

    /**
     * Serialises a sparse matrix to a byte array
     */
    uint8_t *sparseMatrixToBytes(const SparseMatrix<double> &sparse) {
        // TODO is this sparse -> dense conversion costly?
        return matrixToBytes(sparse);
    }

    /**
     * Deserialises a byte array to a matrix
     */
    MatrixXd bytesToMatrix(uint8_t *byteArray, long rows, long columns) {
        auto doubleArray = reinterpret_cast<double *>(&byteArray[0]);

        Map<MatrixXd> mat(&doubleArray[0], rows, columns);

        return mat;
    }

    /**
     * Deserialises a byte array to a sparse matrix
     */
    SparseMatrix<double> bytesToSparseMatrix(uint8_t *byteArray, long rows, long columns) {
        // TODO is this dense -> sparse conversion costly?
        MatrixXd dense = bytesToMatrix(byteArray, rows, columns);

        SparseMatrix<double> sparse;
        sparse = dense.sparseView(0.01, 0.001);

        return sparse;
    }

    /**
     * Writes a matrix to state
     */
    void writeMatrixState(FaasmMemory *memory, const char *key, const MatrixXd &matrix) {
        size_t nBytes = matrix.rows() * matrix.cols() * sizeof(double);
        uint8_t *serialisedData = matrixToBytes(matrix);

        memory->writeState(key, serialisedData, nBytes);

        delete[] serialisedData;
    }

    /**
     * Reads a matrix from state
     */
    MatrixXd readMatrixFromState(FaasmMemory *memory, const char *key, long rows, long cols) {
        size_t nBytes = rows * cols * sizeof(double);

        auto buffer = new uint8_t[nBytes];
        memory->readState(key, buffer, nBytes);

        Eigen::MatrixXd deserialised = bytesToMatrix(buffer, rows, cols);
        delete[] buffer;
        return deserialised;
    }

    long matrixByteIndex(long row, long col, long nRows) {
        // Work out the position of this element
        // Note that matrices are stored in column-major order by default
        long byteIdx = ((col * nRows) + row) * sizeof(double);

        return byteIdx;
    }

    /** 
     * Updates a specific element in state 
     */
    void writeMatrixStateElement(FaasmMemory *memory, const char *key, const MatrixXd &matrix, long row, long col) {
        // Work out the position of this element
        // Note that matrices are stored in column-major order by default
        long byteIdx = matrixByteIndex(row, col, matrix.rows());

        double value = matrix(row, col);
        auto byteValue = reinterpret_cast<uint8_t *>(&value);
        size_t nBytes = sizeof(double);

        memory->writeStateOffset(key, (size_t) byteIdx, byteValue, nBytes);
    }

    /**
     * Reads a subset of full columns from state (note that column numbers are inclusive
     */
    MatrixXd readMatrixColumnsFromState(FaasmMemory *memory, const char *key, long colStart, long colEnd, long nRows) {
        // Note, inclusive
        long nCols = colEnd - colStart + 1;

        long startIdx = matrixByteIndex(0, colStart, nRows);
        long endIdx = matrixByteIndex(nRows, colEnd, nRows);

        long bufferLen = endIdx - startIdx;
        auto buffer = new uint8_t[bufferLen];

        memory->readStateOffset(key, startIdx, buffer, bufferLen);

        MatrixXd result = bytesToMatrix(buffer, nRows, nCols);
        delete[] buffer;
        return result;
    }

    /**
     * Shuffles the columns of a matrix
     */
    void shuffleMatrixColumns(MatrixXd &matrix) {
        int nCols = (int) matrix.cols();

        for (int i = nCols - 1; i > 0; i--) {
            int r = randomInteger(0, nCols - 1);
            matrix.col(i).swap(matrix.col(r));
        }
    }

    /**
     * Shuffles the columns of two matrices in the same order
     */
    void shufflePairedMatrixColumns(MatrixXd &a, MatrixXd &b) {
        int nCols = (int) a.cols();

        for (int i = nCols - 1; i > 0; i--) {
            int r = randomInteger(0, nCols - 1);
            a.col(i).swap(a.col(r));
            b.col(i).swap(b.col(r));
        }
    }

    /**
     * Prints the given matrix
     */
    void printMatrix(MatrixXd &mat) {
        for (int r = 0; r < mat.rows(); r++) {
            for (int c = 0; c < mat.cols(); c++) {
                double val = mat.coeff(r, c);
                printf("%.2f, ", val);
            }

            printf("\n");
        }
    }

    /**
     * Sums the squared error between two vectors when the value in the first is non-zero
     */
    double calculateSquaredError(const MatrixXd &a, const MatrixXd &b) {
        MatrixXd diff = a - b;

        double squaredError = 0;
        for (long r = 0; r < diff.rows(); r++) {
            for (long c = 0; c < diff.cols(); c++) {
                double e = diff.coeff(r, c);
                squaredError += pow(e, 2);
            }
        }

        return squaredError;
    }

    /**
     * Finds the mean squared error between two matrices
     */
     double calculateRootMeanSquaredError(const MatrixXd &a, const MatrixXd &b) {
        double squaredError = calculateSquaredError(a, b);

        long nElements = a.cols() * a.rows();
        double rmse = sqrt(squaredError / nElements);
        return rmse;
     }
}
