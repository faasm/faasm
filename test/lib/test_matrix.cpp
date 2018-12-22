#include <catch/catch.hpp>
#include "utils.h"

#include "faasm/matrix.h"

#include <infra/infra.h>

#include <iostream>

using namespace Eigen;

namespace tests {
    TEST_CASE("Test sparse matrix generation", "[matrix]") {
        const SparseMatrix<double> actual = faasm::randomSparseMatrix(10, 5, 0.4);

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

    TEST_CASE("Test matrix to redis round trip", "[matrix]") {
        redisQueue.flushAll();

        MatrixXd mat = buildDummyMatrix();

        // Initialise dummy faasm memory
        faasm::FaasmMemory mem;

        // Write to a dummy key
        const char *stateKey = "test_matrix_state";
        faasm::writeMatrixToState(&mem, stateKey, mat);

        // Retrieve from redis and check it's still the same
        const MatrixXd afterState = faasm::readMatrixFromState(&mem, stateKey, 2, 3);

        REQUIRE(afterState.rows() == 2);
        REQUIRE(afterState.cols() == 3);
        REQUIRE(afterState == mat);
    }

    TEST_CASE("Test updating matrix element in state", "[matrix]") {
        redisQueue.flushAll();

        MatrixXd mat = buildDummyMatrix();

        // Initialise dummy faasm memory
        faasm::FaasmMemory mem;

        // Write full state to a dummy key
        const char *stateKey = "test_matrix_elem_state";
        faasm::writeMatrixToState(&mem, stateKey, mat);

        // Update the matrix in memory
        mat(0, 2) = 3.3;
        mat(1, 1) = 10.5;

        // Update a single element
        faasm::writeMatrixToStateElement(&mem, stateKey, mat, 0, 2);
        faasm::writeMatrixToStateElement(&mem, stateKey, mat, 1, 1);

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
        redisQueue.flushAll();

        long nRows = 4;
        long nCols = 5;
        MatrixXd mat(nRows, nCols);
        mat << 1, 2, 3, 4, 5,
                6, 7, 8, 9, 10,
                11, 12, 13, 14, 15,
                16, 17, 18, 19, 20;

        // Initialise dummy faasm memory
        faasm::FaasmMemory mem;

        // Write full state to a dummy key
        const char *stateKey = "test_matrix_cols_state";
        faasm::writeMatrixToState(&mem, stateKey, mat);

        // Read a subset of columns (exclusive)
        long startCol = 1;
        long endCol = 4;
        MatrixXd actual = faasm::readMatrixColumnsFromState(&mem, stateKey, nCols, startCol, endCol, nRows);

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
        MatrixXd mat = faasm::randomSparseMatrix(10, 20, 0.4);
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

    TEST_CASE("Test calculating hinge loss", "[matrix]") {
        // Some correctly classified (i.e. correct sign), some not
        MatrixXd matA(1, 4);
        matA << -2.3, 1.1, -3.3, 0.0;

        MatrixXd matB(1, 4);
        matB << 1.0, -1.0, -1.0, 1.0;

        // Error will be zero unless the signs are wrong, or the product of the
        // two is less than 1, so the first, second and fourth elements contribute
        double expected = (1 - (1.0 * -2.3)) + (1 - (-1.0 * 1.1)) + (1 - (0.0 * 1.0));
        double actual = faasm::calculateHingeError(matA, matB);

        REQUIRE(actual == expected);
    }

    TEST_CASE("Test sparse matrix round trip", "[matrix]") {
        redisQueue.flushAll();

        SparseMatrix<double> mat = faasm::randomSparseMatrix(5, 10, 0.4);

        faasm::FaasmMemory mem;

        const char *key = "sparse_trip_test";
        faasm::writeSparseMatrixToState(&mem, key, mat);

        SparseMatrix<double> actual = faasm::readSparseMatrixFromState(&mem, key);
        checkSparseMatrixEquality(mat, actual);
    }

    void checkSparseMatrixRoundTrip(int rows, int cols, int colStart, int colEnd) {
        redisQueue.flushAll();

        const char *key = "sparse_trip_offset_test";

        SparseMatrix<double> mat = faasm::randomSparseMatrix(rows, cols, 0.4);
        faasm::FaasmMemory mem;
        faasm::writeSparseMatrixToState(&mem, key, mat);

        SparseMatrix<double> expected = mat.block(0, colStart, rows, colEnd - colStart);

        // Read a subsection
        SparseMatrix<double> actual = faasm::readSparseMatrixColumnsFromState(&mem, key, colStart, colEnd);
        checkSparseMatrixEquality(actual, expected);
    }

    TEST_CASE("Test sparse matrix offset multiple columns", "[matrix]") {
        checkSparseMatrixRoundTrip(10, 15, 3, 10);
    }

    TEST_CASE("Test sparse matrix offset single column", "[matrix]") {
        checkSparseMatrixRoundTrip(10, 15, 10, 11);
    }

    TEST_CASE("Test sparse matrix offset last column", "[matrix]") {
        checkSparseMatrixRoundTrip(10, 15, 14, 15);
    }

    TEST_CASE("Test sparse matrix offset first column", "[matrix]") {
        checkSparseMatrixRoundTrip(10, 15, 0, 1);
    }
}