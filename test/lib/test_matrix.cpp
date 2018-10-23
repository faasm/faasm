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

    void checkDoubleBytes(double expected, uint8_t* byteArrayStart) {
        double actual = *reinterpret_cast<double*>(&byteArrayStart[0]);
        REQUIRE(expected == actual);
    }

    TEST_CASE("Test matrix to bytes round trip", "[matrix]") {
        infra::Redis redis;
        redis.flushAll();

        MatrixXd initial(2, 3);
        initial<< 1, 2, 3,
            4, 5, 6;

        REQUIRE(initial.rows() == 2);
        REQUIRE(initial.cols() == 3);

        long nBytes = 2 * 3 * sizeof(double);
        uint8_t *byteArray = faasm::matrixToBytes(initial);

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
        REQUIRE(matrixOut == initial);

        delete[] byteArray;
    }

    TEST_CASE("Test matrix to redis round trip", "[matrix]") {
        infra::Redis redis;
        redis.flushAll();
        
        MatrixXd initial(2, 3);
        initial<< 1, 2, 3,
                4, 5, 6;

        // Initialise dummy faasm memory
        uint8_t dummy[2];
        faasm::FaasmMemory mem(dummy, dummy, dummy, dummy);
        
        // Write to a dummy key
        const char* stateKey = "test_matrix_state";
        faasm::writeMatrixState(&mem, stateKey, initial);

        // Retrieve from redis and check it's still the same
        const MatrixXd afterState = faasm::readMatrixFromState(&mem, stateKey, 2, 3);

        REQUIRE(afterState.rows() == 2);
        REQUIRE(afterState.cols() == 3);
        REQUIRE(afterState == initial);
    }
}