#ifndef FAASM_MATRIX_H
#define FAASM_MATRIX_H

#include "faasm.h"

#include <eigen3/Eigen/Core>
#include <eigen3/Eigen/SparseCore>
#include <random>

using namespace Eigen;

namespace faasm {

    double *byteArrayToDoubles(uint8_t *byteArray, long byteArrayLen) {
        size_t nDoubles = byteArrayLen / sizeof(double);
        auto *doubleArray = new double[nDoubles];

        std::copy(doubleArray, doubleArray + nDoubles, byteArray);

        return doubleArray;
    }

    MatrixXd randomSparseMatrix(int rows, int cols) {
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
     * Deserialises a byte array to a matrix
     */
    MatrixXd bytesToMatrix(uint8_t *byteArray, long rows, long columns) {
        auto doubleArray = reinterpret_cast<double *>(&byteArray[0]);

        Map<MatrixXd> mat(&doubleArray[0], rows, columns);

        return mat;
    }

    /**
     * Writes a matrix to state
     */
    void writeMatrixState(FaasmMemory* memory, const char* key, const MatrixXd &matrix) {
        size_t nBytes = matrix.rows() * matrix.cols() * sizeof(double);
        uint8_t *serialisedData = matrixToBytes(matrix);

        memory->writeState(key, serialisedData, nBytes);
    }

    /**
     * Reads a matrix from state
     */
    MatrixXd readMatrixFromState(FaasmMemory* memory, const char* key, long rows, long cols) {
        size_t nBytes = rows * cols * sizeof(double);

        uint8_t buffer[nBytes];
        memory->readState(key, buffer, nBytes);

        Eigen::MatrixXd deserialised = bytesToMatrix(buffer, rows, 1);
        return deserialised;
    }
}

#endif
