#include <catch/catch.hpp>

#include "faasm/matrix.h"
#include <infra/infra.h>

#include <iostream>

using namespace Eigen;

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

    TEST_CASE("Test sparse matrix generation", "[matrix]") {
        const SparseMatrix<double> actual = faasm::randomSparseMatrix(10, 5);

        REQUIRE(actual.rows() == 10);
        REQUIRE(actual.cols() == 5);

        // Number of non-zero elements should be much lower than the max
        REQUIRE(actual.nonZeros() < 40);
    }

    void checkDoubleBytes(double expected, uint8_t *byteArrayStart) {
        double actual = *reinterpret_cast<double *>(&byteArrayStart[0]);
        REQUIRE(expected == actual);
    }

    MatrixXd buildDummyMatrix() {
        MatrixXd mat(2, 3);
        mat << 1, 2, 3,
                4, 5, 6;

        return mat;
    }

    TEST_CASE("Test matrix to bytes round trip", "[matrix]") {
        MatrixXd mat = buildDummyMatrix();

        REQUIRE(mat.rows() == 2);
        REQUIRE(mat.cols() == 3);

        uint8_t *byteArray = faasm::matrixToBytes(mat);

        // Double check size assumption
        REQUIRE(sizeof(double) == 8);

        // Note, column-major order
        checkDoubleBytes(1, &byteArray[0]);
        checkDoubleBytes(4, &byteArray[8]);
        checkDoubleBytes(2, &byteArray[16]);
        checkDoubleBytes(5, &byteArray[24]);
        checkDoubleBytes(3, &byteArray[32]);
        checkDoubleBytes(6, &byteArray[40]);

        // Rebuild matrix from array and check it has survived round trip
        MatrixXd matrixOut = faasm::bytesToMatrix(byteArray, 2, 3);
        REQUIRE(matrixOut.rows() == 2);
        REQUIRE(matrixOut.cols() == 3);
        REQUIRE(matrixOut == mat);

        delete[] byteArray;
    }

    TEST_CASE("Test sparse matrix to bytes round trip", "[matrix]") {
        SparseMatrix<double> mat = faasm::randomSparseMatrix(10, 5);
        uint8_t *byteArray = faasm::sparseMatrixToBytes(mat);

        SparseMatrix<double> matrixOut = faasm::bytesToSparseMatrix(byteArray, 10, 5);

        checkSparseMatrixEquality(mat, matrixOut);

        delete[] byteArray;
    }

    TEST_CASE("Test matrix to redis round trip", "[matrix]") {
        infra::Redis redis;
        redis.flushAll();

        MatrixXd mat = buildDummyMatrix();

        // Initialise dummy faasm memory
        faasm::FaasmMemory mem;

        // Write to a dummy key
        const char *stateKey = "test_matrix_state";
        faasm::writeMatrixState(&mem, stateKey, mat);

        // Retrieve from redis and check it's still the same
        const MatrixXd afterState = faasm::readMatrixFromState(&mem, stateKey, 2, 3);

        REQUIRE(afterState.rows() == 2);
        REQUIRE(afterState.cols() == 3);
        REQUIRE(afterState == mat);
    }

    TEST_CASE("Test updating matrix element in state", "[matrix]") {
        infra::Redis redis;
        redis.flushAll();

        MatrixXd mat = buildDummyMatrix();

        // Initialise dummy faasm memory
        faasm::FaasmMemory mem;

        // Write full state to a dummy key
        const char *stateKey = "test_matrix_elem_state";
        faasm::writeMatrixState(&mem, stateKey, mat);

        // Update the matrix in memory
        mat(0, 2) = 3.3;
        mat(1, 1) = 10.5;

        // Update a single element
        faasm::writeMatrixStateElement(&mem, stateKey, mat, 0, 2);
        faasm::writeMatrixStateElement(&mem, stateKey, mat, 1, 1);

        // Retrieve from redis and check it matches the one in memory
        const MatrixXd afterState = faasm::readMatrixFromState(&mem, stateKey, 2, 3);

        REQUIRE(afterState.rows() == 2);
        REQUIRE(afterState.cols() == 3);
        REQUIRE(afterState == mat);

        // Explicitly check set values
        REQUIRE(afterState(0, 2) == 3.3);
        REQUIRE(afterState(1, 1) == 10.5);
    }

    TEST_CASE("Test reading columns from state", "[matrix]") {
        infra::Redis redis;
        redis.flushAll();

        long nRows = 4;
        MatrixXd mat(nRows, 5);
        mat << 1, 2, 3, 4, 5,
                6, 7, 8, 9, 10,
                11, 12, 13, 14, 15,
                16, 17, 18, 19, 20;

        // Initialise dummy faasm memory
        faasm::FaasmMemory mem;

        // Write full state to a dummy key
        const char *stateKey = "test_matrix_cols_state";
        faasm::writeMatrixState(&mem, stateKey, mat);

        // Read a subset of columns
        long startCol = 1;
        long endCol = 3;
        MatrixXd actual = faasm::readMatrixColumnsFromState(&mem, stateKey, startCol, endCol, nRows);

        REQUIRE(actual.rows() == nRows);
        REQUIRE(actual.cols() == 3);

        MatrixXd expected(nRows, 3);
        expected << 2, 3, 4,
                7, 8, 9,
                12, 13, 14,
                17, 18, 19;

        REQUIRE(actual == expected);
    }

    TEST_CASE("Test shuffling matrix", "[matrix]") {
        MatrixXd mat = faasm::randomSparseMatrix(10, 20);
        MatrixXd copy = mat;

        faasm::shuffleMatrixColumns(mat);

        REQUIRE(mat.rows() == 10);
        REQUIRE(mat.cols() == 20);

        // Not technically true but high probability
        REQUIRE(copy != mat);
    }

    TEST_CASE("Test shuffling paired matrices", "[matrix]") {
        MatrixXd matA(2, 10);
        MatrixXd matB(1, 10);

        matA << 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
                11, 12, 13, 14, 15, 16, 17, 18, 19, 20;

        matB << 111, 222, 333, 444, 555, 666, 777, 888, 999, 1111;

        MatrixXd copyA = matA;
        MatrixXd copyB = matB;

        faasm::shufflePairedMatrixColumns(matA, matB);

        // First check shuffling has done its job (small chance this will fail)
        REQUIRE(copyA != matA);
        REQUIRE(copyB != matB);

        // Compare known columns in both to check that they still match up
        double valA = 5;
        double valB = 555;

        int idxA = 0;
        for (int i = 0; i < 10; i++) {
            double thisVal = matA.coeff(0, i);
            if (thisVal == valA) {
                idxA = i;
                break;
            }
        }

        REQUIRE(matB.coeff(0, idxA) == valB);
    }

    TEST_CASE("Test root mean squared error", "[matrix]") {
        MatrixXd matA(1, 3);
        matA << 8.5, 10.1, 15.5;

        MatrixXd matB(1, 3);
        matB << 2.5, 1.3, 16.6;

        double a = std::pow(8.5 - 2.5, 2);
        double b = std::pow(10.1 - 1.3, 2);
        double c = std::pow(15.5 - 16.6, 2);

        double expected = sqrt((a + b + c) / 3.0);

        double actual = faasm::calculateRootMeanSquaredError(matA, matB);

        REQUIRE(actual == expected);
    }

    TEST_CASE("Test calculating squared error", "[matrix]") {
        MatrixXd matA(1, 4);
        matA << 11.1, 0.0, 12.2, 13.3;

        MatrixXd matB(1, 4);
        matB << 0.0, 21.1, 20.0, 19.9;

        double a = std::pow(11.1 - 0.0, 2);
        double b = std::pow(0.0 - 21.1, 2);
        double c = std::pow(12.2 - 20.0, 2);
        double d = std::pow(13.3 - 19.9, 2);

        double actual = faasm::calculateSquaredError(matA, matB);
        REQUIRE(actual == a + b + c + d);
    }

    TEST_CASE("Test sparse matrix round trip", "[matrix]") {
        infra::Redis redis;
        redis.flushAll();

        SparseMatrix<double> mat = faasm::randomSparseMatrix(5, 10);

        faasm::FaasmMemory mem;

        const char *key = "sparse_trip_test";

        faasm::writeSparseMatrixToState(&mem, mat, key);

        SparseMatrix<double> actual = faasm::readSparseMatrixFromState(&mem, key);

        checkSparseMatrixEquality(mat, actual);
    }

}