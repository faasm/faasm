#include <catch/catch.hpp>

#include <faasm/matrix.h>
#include <infra/infra.h>

using namespace Eigen;

namespace tests {

    TEST_CASE("Test sparse matrix generation", "[matrix]") {
        const MatrixXd &actual = faasm::randomSparseMatrix(10, 5);

        REQUIRE(actual.rows() == 10);
        REQUIRE(actual.cols() == 5);
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

    faasm::FaasmMemory buildDummyMemory() {
        uint8_t dummy[2];
        faasm::FaasmMemory mem(dummy, dummy, dummy, dummy);

        return mem;
    }

    TEST_CASE("Test matrix to bytes round trip", "[matrix]") {
        infra::Redis redis;
        redis.flushAll();

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
        infra::Redis redis;
        redis.flushAll();

        MatrixXd mat = buildDummyMatrix();

        // Initialise dummy faasm memory
        faasm::FaasmMemory mem = buildDummyMemory();

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
        faasm::FaasmMemory mem = buildDummyMemory();

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
        faasm::FaasmMemory mem = buildDummyMemory();

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

    TEST_CASE("Test generating sparse random matrix", "[matrix]") {
        int rows = 2;
        int cols = 5;
        const MatrixXd &actual = faasm::randomSparseMatrix(rows, cols);

        REQUIRE(actual.rows() == rows);
        REQUIRE(actual.cols() == cols);

        // At least half of the values should be zero
        int zeroCount = 0;
        for (int r = 0; r < rows; r++) {
            for (int c = 0; c < cols; c++) {
                double thisVal = actual(r, c);
                if (thisVal == 0) {
                    zeroCount++;
                }
            }
        }

        REQUIRE(zeroCount > 5);
    }

    TEST_CASE("Test shuffling matrix", "[matrix]") {
        MatrixXd mat = faasm::randomSparseMatrix(10, 20);
        MatrixXd shuffled = faasm::shuffleMatrixColumns(mat);

        REQUIRE(shuffled.rows() == mat.rows());
        REQUIRE(shuffled.cols() == mat.cols());

        // Not technically always correct, but unlikely when the matrix is large enough
        REQUIRE(mat != shuffled);
    }

}