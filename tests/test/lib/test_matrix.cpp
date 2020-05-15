#include <catch/catch.hpp>
#include "utils.h"

#include "faasm/matrix.h"

#include <redis/Redis.h>
#include <state/State.h>

#include <iostream>
#include <emulator/emulator.h>
#include <util/state.h>
#include <state/StateServer.h>

using namespace Eigen;

namespace tests {
    TEST_CASE("Test sparse matrix generation", "[matrix]") {
        const SparseMatrix<double> actual = faasm::randomSparseMatrix(10, 5, 0.4);

        REQUIRE(actual.rows() == 10);
        REQUIRE(actual.cols() == 5);

        // Number of non-zero elements should be much lower than the max
        REQUIRE(actual.nonZeros() < 40);
    }

    MatrixXd buildDummyMatrix() {
        MatrixXd mat(2, 3);
        mat << 1, 2, 3,
                4, 5, 6;

        return mat;
    }

    static size_t setUpDummyStateServer(DummyStateServer &server, const char *stateKey, const MatrixXd &mat) {
        size_t nBytes = getMatrixStateSize(mat);

        std::vector<uint8_t> remoteValue(nBytes, 0);
        server.dummyUser = getEmulatorUser();
        server.dummyKey = stateKey;
        server.dummyData = remoteValue;

        return nBytes;
    }

    TEST_CASE("Test matrix to remote state round trip", "[matrix]") {
        cleanSystem();

        DummyStateServer server;
        const char *stateKey = "test_matrix_state";
        MatrixXd mat = buildDummyMatrix();
        size_t nBytes = setUpDummyStateServer(server, stateKey, mat);

        // One push, one pull
        server.start(2);

        // Write locally and push
        faasm::writeMatrixToState(stateKey, mat, true);

        // Check it exists locally
        state::State &localState = state::getGlobalState();
        REQUIRE(localState.getKVCount() == 1);
        REQUIRE(server.getLocalKvValue().size() == nBytes);

        // Delete locally
        localState.deleteKVLocally(server.dummyUser, server.dummyKey);
        REQUIRE(localState.getKVCount() == 0);

        // Retrieve from state and make sure it's pulled
        Map<const MatrixXd> afterState = faasm::readMatrixFromState(stateKey, 2, 3, true);
        REQUIRE(localState.getKVCount() == 1);

        // Check the matrix
        REQUIRE(afterState.rows() == 2);
        REQUIRE(afterState.cols() == 3);
        REQUIRE(afterState == mat);

        server.wait();
    }

    TEST_CASE("Test updating matrix element in state", "[matrix]") {
        cleanSystem();

        MatrixXd mat = buildDummyMatrix();
        DummyStateServer server;
        const char *stateKey = "test_matrix_elem_state";

        // Set up remote server with matrix
        size_t nBytes = setUpDummyStateServer(server, stateKey, mat);

        // Two pushes, one chunked push (implicit pull)
        server.start(4);

        // Write to state locally
        faasm::writeMatrixToState(stateKey, mat, true);

        // Update the matrix in memory
        mat(0, 2) = 3.3;
        mat(1, 1) = 10.5;

        // Write a couple of elements
        faasm::writeMatrixToStateElement(stateKey, mat, 0, 2, false);
        faasm::writeMatrixToStateElement(stateKey, mat, 1, 1, true);

        // Retrieve remote state into a new memory location
        MatrixXd afterState(2, 3);
        faasm::readMatrixFromState(stateKey, afterState.data(), 2, 3, true);

        // Check things match up
        REQUIRE(afterState.rows() == 2);
        REQUIRE(afterState.cols() == 3);
        REQUIRE(afterState == mat);

        // Explicitly check set values
        REQUIRE(afterState(0, 2) == 3.3);
        REQUIRE(afterState(1, 1) == 10.5);

        server.wait();
    }

    void checkReadingMatrixColumnsFromState(bool local) {
        cleanSystem();

        const char *stateKey = "test_matrix_cols_state";

        long nRows = 4;
        long nCols = 5;
        MatrixXd mat(nRows, nCols);

        // If async, more messages
        DummyStateServer server;
        bool pushPull = !local;
        if (pushPull) {
            size_t nBytes = setUpDummyStateServer(server, stateKey, mat);
            server.start(2);
        }

        mat << 1, 2, 3, 4, 5,
                6, 7, 8, 9, 10,
                11, 12, 13, 14, 15,
                16, 17, 18, 19, 20;

        // Write full state to a dummy key
        faasm::writeMatrixToState(stateKey, mat, pushPull);

        // Read a subset of columns (exclusive)
        long startCol = 1;
        long endCol = 4;
        Map<const MatrixXd> actual = faasm::readMatrixColumnsFromState(stateKey, nCols, startCol, endCol, nRows,
                                                                       pushPull);

        REQUIRE(actual.rows() == nRows);
        REQUIRE(actual.cols() == 3);

        MatrixXd expected(nRows, 3);
        expected << 2, 3, 4,
                7, 8, 9,
                12, 13, 14,
                17, 18, 19;

        REQUIRE(actual == expected);

        if (pushPull) {
            server.wait();
        }
    }

    TEST_CASE("Test reading columns from state remotely", "[matrix]") {
        checkReadingMatrixColumnsFromState(false);
    }

    TEST_CASE("Test reading columns from state locally", "[matrix]") {
        checkReadingMatrixColumnsFromState(true);
    }

    TEST_CASE("Test shuffling matrix", "[matrix]") {
        MatrixXd mat = faasm::randomSparseMatrix(10, 20, 0.4);
        MatrixXd copy = mat;

        faasm::shuffleMatrixColumns(mat);

        REQUIRE(mat.rows() == 10);
        REQUIRE(mat.cols() == 20);

        // Not technically always true but high probability
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
        cleanSystem();
        SparseMatrix<double> mat = faasm::randomSparseMatrix(5, 10, 0.4);

        const char *key = "sparse_trip_test";
        faasm::writeSparseMatrixToState(key, mat, true);

        Map<const SparseMatrix<double>> actual = faasm::readSparseMatrixFromState(key, true);
        checkSparseMatrixEquality(mat, actual);
    }

    void doRemoteSparseMatrixRoundTripCheck(int rows, int cols, int colStart, int colEnd) {
        cleanSystem();

        int nMessages = 8;

        const char *key = "sparse_trip_offset_test";
        SparseMatrix<double> mat = faasm::randomSparseMatrix(rows, cols, 0.7);
        state::State remoteState(LOCALHOST);

        // Run state server in the background
        std::thread serverThread([&key, &mat, &remoteState, nMessages] {
            state::StateServer stateServer(remoteState);

            // Write matrix to state to master in this thread
            faasm::writeSparseMatrixToState(key, mat, false);

            // Process enough messages
            int processedMessages = 0;
            while (processedMessages < nMessages) {
                processedMessages += stateServer.poll();
            }

            // Shut down
            stateServer.close();
        });

        // Give it time to start
        ::usleep(1000 * 500);

        // Get subsection from the matrix
        SparseMatrix<double> expected = mat.block(0, colStart, rows, colEnd - colStart);

        // Get from state
        Map<const SparseMatrix<double>> actual = faasm::readSparseMatrixColumnsFromState(key, colStart, colEnd, true);
        checkSparseMatrixEquality(actual, expected);

        // Read the whole thing and check
        Map<const SparseMatrix<double>> actualFull = faasm::readSparseMatrixFromState(key, true);
        checkSparseMatrixEquality(actualFull, mat);

        // Wait for server
        if (serverThread.joinable()) {
            serverThread.join();
        }
    }

    void doLocalSparseMatrixRoundTripCheck(int rows, int cols, int colStart, int colEnd) {
        cleanSystem();

        const char *key = "sparse_trip_offset_test";
        SparseMatrix<double> mat = faasm::randomSparseMatrix(rows, cols, 0.7);

        // Write matrix to state to master in this thread
        faasm::writeSparseMatrixToState(key, mat, false);

        // Get subsection from the matrix
        SparseMatrix<double> expected = mat.block(0, colStart, rows, colEnd - colStart);

        // Get from state
        Map<const SparseMatrix<double>> actual = faasm::readSparseMatrixColumnsFromState(key, colStart, colEnd, false);
        checkSparseMatrixEquality(actual, expected);

        // Read the whole thing and check
        Map<const SparseMatrix<double>> actualFull = faasm::readSparseMatrixFromState(key, false);
        checkSparseMatrixEquality(actualFull, mat);
    }

    void checkSparseMatrixRoundTrip(int rows, int cols, int colStart, int colEnd) {
        // Do both locally and remotely
        doRemoteSparseMatrixRoundTripCheck(rows, cols, colStart, colEnd);
        doLocalSparseMatrixRoundTripCheck(rows, cols, colStart, colEnd);
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

    TEST_CASE("Test big sparse matrix", "[matrix]") {
        checkSparseMatrixRoundTrip(4000, 300, 123, 150);
    }
}