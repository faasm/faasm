#include "utils.h"
#include <catch2/catch.hpp>

#include "faasm/matrix.h"

#include <faabric/redis/Redis.h>
#include <faabric/state/State.h>

#include <emulator/emulator.h>
#include <faabric/state/DummyStateServer.h>
#include <faabric/util/network.h>
#include <faabric/util/state.h>

using namespace Eigen;
using namespace faabric::state;

namespace tests {
TEST_CASE("Test byte offsets for matrix elements", "[matrix]")
{
    cleanSystem();

    // 0,0 should always be zero
    REQUIRE(faasm::getChunkSizeUpToMatrixElement(0, 0, 10) == 0);
    REQUIRE(faasm::getChunkSizeUpToMatrixElement(0, 0, 1000) == 0);

    // Second element of first column should always be one element off
    REQUIRE(faasm::getChunkSizeUpToMatrixElement(1, 0, 10) == sizeof(double));
    REQUIRE(faasm::getChunkSizeUpToMatrixElement(1, 0, 1000) == sizeof(double));

    // First element of second column should always be one column length off
    REQUIRE(faasm::getChunkSizeUpToMatrixElement(0, 1, 10) ==
            10 * sizeof(double));
    REQUIRE(faasm::getChunkSizeUpToMatrixElement(0, 1, 1000) ==
            1000 * sizeof(double));

    // Check an arbitrary offset
    REQUIRE(faasm::getChunkSizeUpToMatrixElement(5, 6, 1000) ==
            (6 * 1000 + 5) * sizeof(double));

    // Check for a row matrix
    REQUIRE(faasm::getChunkSizeUpToMatrixElement(0, 2, 1) ==
            2 * sizeof(double));
}

TEST_CASE("Test sparse matrix generation", "[matrix]")
{
    cleanSystem();

    const SparseMatrix<double> actual = faasm::randomSparseMatrix(10, 5, 0.4);

    REQUIRE(actual.rows() == 10);
    REQUIRE(actual.cols() == 5);

    // Number of non-zero elements should be much lower than the max
    REQUIRE(actual.nonZeros() < 40);
}

MatrixXd buildDummyMatrix()
{
    MatrixXd mat(2, 3);
    mat << 1, 2, 3, 4, 5, 6;

    return mat;
}

static size_t setUpDummyStateServer(DummyStateServer& server,
                                    const char* stateKey,
                                    const MatrixXd& mat)
{
    size_t nBytes = getMatrixStateSize(mat);

    std::vector<uint8_t> remoteValue(nBytes, 0);
    server.dummyUser = getEmulatorUser();
    server.dummyKey = stateKey;
    server.dummyData = remoteValue;

    return nBytes;
}

TEST_CASE("Test matrix to remote state round trip", "[matrix]")
{
    cleanSystem();

    DummyStateServer server;
    const char* stateKey = "test_matrix_state";
    MatrixXd mat = buildDummyMatrix();
    size_t nBytes = setUpDummyStateServer(server, stateKey, mat);

    server.start();

    // Write locally and push
    faasm::writeMatrixToState(stateKey, mat, true);

    // Check it exists locally
    faabric::state::State& localState = faabric::state::getGlobalState();
    REQUIRE(localState.getKVCount() == 1);
    REQUIRE(server.getLocalKvValue().size() == nBytes);

    // Delete locally
    localState.deleteKVLocally(server.dummyUser, server.dummyKey);
    REQUIRE(localState.getKVCount() == 0);

    // Retrieve from state and make sure it's pulled
    Map<const MatrixXd> afterState =
      faasm::readMatrixFromState(stateKey, 2, 3, true);
    REQUIRE(localState.getKVCount() == 1);

    // Check the matrix
    REQUIRE(afterState.rows() == 2);
    REQUIRE(afterState.cols() == 3);
    REQUIRE(afterState == mat);

    server.stop();
}

void checkReadingMatrixColumnsFromState(bool local)
{
    cleanSystem();

    const char* stateKey = "test_matrix_cols_state";

    long nRows = 4;
    long nCols = 5;
    MatrixXd mat(nRows, nCols);

    // If async, more messages
    DummyStateServer server;
    bool pushPull = !local;
    if (pushPull) {
        setUpDummyStateServer(server, stateKey, mat);
        server.start();
    }

    mat << 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
      20;

    // Write full state to a dummy key
    faasm::writeMatrixToState(stateKey, mat, pushPull);

    // Read a subset of columns (exclusive)
    long startCol = 1;
    long endCol = 4;
    Map<const MatrixXd> actual = faasm::readMatrixColumnsFromState(
      stateKey, nCols, startCol, endCol, nRows, pushPull);

    REQUIRE(actual.rows() == nRows);
    REQUIRE(actual.cols() == 3);

    MatrixXd expected(nRows, 3);
    expected << 2, 3, 4, 7, 8, 9, 12, 13, 14, 17, 18, 19;

    REQUIRE(actual == expected);

    if (pushPull) {
        server.stop();
    }
}

TEST_CASE("Test reading columns from state remotely", "[matrix]")
{
    checkReadingMatrixColumnsFromState(false);
}

TEST_CASE("Test reading columns from state locally", "[matrix]")
{
    checkReadingMatrixColumnsFromState(true);
}

TEST_CASE("Test shuffling matrix", "[matrix]")
{
    cleanSystem();

    MatrixXd mat = faasm::randomSparseMatrix(10, 20, 0.4);
    MatrixXd copy = mat;

    faasm::shuffleMatrixColumns(mat);

    REQUIRE(mat.rows() == 10);
    REQUIRE(mat.cols() == 20);

    // Not technically always true but high probability
    REQUIRE(copy != mat);
}

TEST_CASE("Test shuffling paired matrices", "[matrix]")
{
    cleanSystem();

    MatrixXd matA(2, 10);
    MatrixXd matB(1, 10);

    matA << 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
      20;

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

TEST_CASE("Test root mean squared error", "[matrix]")
{
    cleanSystem();

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

TEST_CASE("Test calculating squared error", "[matrix]")
{
    cleanSystem();

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

TEST_CASE("Test calculating hinge loss", "[matrix]")
{
    cleanSystem();

    // Some correctly classified (i.e. correct sign), some not
    MatrixXd matA(1, 4);
    matA << -2.3, 1.1, -3.3, 0.0;

    MatrixXd matB(1, 4);
    matB << 1.0, -1.0, -1.0, 1.0;

    // Error will be zero unless the signs are wrong, or the product of the
    // two is less than 1, so the first, second and fourth elements contribute
    double expected =
      (1 - (1.0 * -2.3)) + (1 - (-1.0 * 1.1)) + (1 - (0.0 * 1.0));
    double actual = faasm::calculateHingeError(matA, matB);

    REQUIRE(actual == expected);
}

TEST_CASE("Test sparse matrix round trip", "[matrix]")
{
    cleanSystem();

    SparseMatrix<double> mat = faasm::randomSparseMatrix(5, 10, 0.4);

    const char* key = "sparse_trip_test";
    faasm::writeSparseMatrixToState(key, mat, true);

    Map<const SparseMatrix<double>> actual =
      faasm::readSparseMatrixFromState(key, true);
    checkSparseMatrixEquality(mat, actual);
}

void doInMemoryStateRoundTripCheck(int rows, int cols, int colStart, int colEnd)
{
    cleanSystem();

    const char* key = "sparse_trip_offset_test";
    SparseMatrix<double> mat = faasm::randomSparseMatrix(rows, cols, 0.7);

    std::string emulatorUser = getEmulatorUser();
    faabric::state::State& globalState = faabric::state::getGlobalState();
    faabric::state::State remoteState(LOCALHOST);
    faabric::state::StateServer stateServer(remoteState);

    REQUIRE(globalState.getThisIP() != remoteState.getThisIP());

    // Run state server in the background
    std::thread serverThread(
      [&emulatorUser, &key, &mat, &remoteState, &stateServer] {
          // Make sure emulator set up properly in this thread
          setEmulatorUser(emulatorUser.c_str());
          setEmulatorState(&remoteState);

          // Write matrix to state to set master as this thread
          faasm::writeSparseMatrixToState(key, mat, false);

          // Process messages. Run the server in _this_ thread.
          stateServer.start(false);
      });

    // Give it time to start
    ::usleep(0.5 * 1000 * 1000);

    // Get subsection from the matrix
    SparseMatrix<double> expected =
      mat.block(0, colStart, rows, colEnd - colStart);

    // Get from state
    Map<const SparseMatrix<double>> actual =
      faasm::readSparseMatrixColumnsFromState(key, colStart, colEnd, true);
    checkSparseMatrixEquality(actual, expected);

    // Read the whole thing and check
    Map<const SparseMatrix<double>> actualFull =
      faasm::readSparseMatrixFromState(key, true);
    checkSparseMatrixEquality(actualFull, mat);

    // Send shutdown message
    stateServer.stop();

    // Wait for server
    if (serverThread.joinable()) {
        serverThread.join();
    }
}

void doRedisStateRoundTripCheck(int rows, int cols, int colStart, int colEnd)
{
    cleanSystem();

    const char* key = "sparse_trip_offset_test";
    SparseMatrix<double> mat = faasm::randomSparseMatrix(rows, cols, 0.7);

    std::string emulatorUser = getEmulatorUser();

    faabric::util::SystemConfig& conf = faabric::util::getSystemConfig();
    std::string originalStateMode = conf.stateMode;
    conf.stateMode = "redis";

    // Write matrix to state
    faasm::writeSparseMatrixToState(key, mat, true);

    // Remove local copies
    faabric::state::getGlobalState().forceClearAll(false);

    // Get subsection from the matrix
    SparseMatrix<double> expected =
      mat.block(0, colStart, rows, colEnd - colStart);

    // Get from state
    Map<const SparseMatrix<double>> actual =
      faasm::readSparseMatrixColumnsFromState(key, colStart, colEnd, false);
    checkSparseMatrixEquality(actual, expected);

    // Read the whole thing and check
    Map<const SparseMatrix<double>> actualFull =
      faasm::readSparseMatrixFromState(key, false);
    checkSparseMatrixEquality(actualFull, mat);

    // Reset state mode
    conf.stateMode = originalStateMode;
}

void doLocalSparseMatrixRoundTripCheck(int rows,
                                       int cols,
                                       int colStart,
                                       int colEnd)
{
    cleanSystem();

    const char* key = "sparse_trip_offset_test";
    SparseMatrix<double> mat = faasm::randomSparseMatrix(rows, cols, 0.7);

    // Write matrix to state
    faasm::writeSparseMatrixToState(key, mat, false);

    // Get subsection from the matrix
    SparseMatrix<double> expected =
      mat.block(0, colStart, rows, colEnd - colStart);

    // Get the subsection from state and check equality
    Map<const SparseMatrix<double>> actual =
      faasm::readSparseMatrixColumnsFromState(key, colStart, colEnd, true);
    checkSparseMatrixEquality(actual, expected);

    // Read the whole thing and check equality
    Map<const SparseMatrix<double>> actualFull =
      faasm::readSparseMatrixFromState(key, true);
    checkSparseMatrixEquality(actualFull, mat);
}

void checkSparseMatrixRoundTrip(int rows, int cols, int colStart, int colEnd)
{
    SECTION("Redis")
    {
        doRedisStateRoundTripCheck(rows, cols, colStart, colEnd);
    }

    SECTION("In memory")
    {
        doInMemoryStateRoundTripCheck(rows, cols, colStart, colEnd);
    }

    SECTION("Local")
    {
        doLocalSparseMatrixRoundTripCheck(rows, cols, colStart, colEnd);
    }
}

TEST_CASE("Test sparse matrix offset multiple columns", "[matrix]")
{
    checkSparseMatrixRoundTrip(10, 15, 3, 10);
}

TEST_CASE("Test sparse matrix offset single column", "[matrix]")
{
    checkSparseMatrixRoundTrip(10, 15, 10, 11);
}

TEST_CASE("Test sparse matrix offset last column", "[matrix]")
{
    checkSparseMatrixRoundTrip(10, 15, 14, 15);
}

TEST_CASE("Test sparse matrix offset first column", "[matrix]")
{
    checkSparseMatrixRoundTrip(10, 15, 0, 1);
}

TEST_CASE("Test big sparse matrix", "[matrix]")
{
    checkSparseMatrixRoundTrip(4000, 300, 123, 150);
}
}
