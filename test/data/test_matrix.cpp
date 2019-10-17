#include <catch/catch.hpp>

#include "utils.h"
#include <data/data.h>

#include <boost/filesystem.hpp>
#include <util/state.h>
#include <util/files.h>
#include <emulator/emulator.h>

using namespace data;
using namespace boost::filesystem;

namespace tests {
    path _setUp() {
        cleanSystem();

        path dummyDir = "/tmp/faasm-test/";
        if (exists(dummyDir)) {
            remove_all(dummyDir);
        }
        return dummyDir;
    }

    void _readFileWriteState(const std::string &user, path dummyDir, const std::string &matrixPart, const std::string &key) {
        redis::Redis &redisState = redis::Redis::getState();

        // Read in bytes
        path filePath = dummyDir;
        filePath.append(matrixPart);
        const std::vector<uint8_t> fileBytes = util::readFileToBytes(filePath.string());

        // Write to state
        const std::string actualKey = util::keyForUser(user, key);
        redisState.set(actualKey, fileBytes);
    }

    TEST_CASE("Check sparse matrix file round trip", "[data]") {
        SparseMatrix<double> mat = faasm::randomSparseMatrix(5, 10, 0.4);

        path dummyDir = _setUp();
        SparseMatrixFileSerialiser s(mat);
        s.writeToFile(dummyDir.string());
        
        // Load sizes and check they still match
        path sizePath = dummyDir;
        sizePath.append("size");
        const std::vector<uint8_t> &sizeBytes = util::readFileToBytes(sizePath.string());
        auto sizes = reinterpret_cast<const SparseSizes *>(sizeBytes.data());

        REQUIRE(sizes->rows == 5);
        REQUIRE(sizes->cols == 10);

        // Read all files in
        SparseMatrix<double> actual = SparseMatrixFileSerialiser::readFromFiles(dummyDir.string());
        checkSparseMatrixEquality(mat, actual);
    }

    TEST_CASE("Check sparse matrix file and state round trip", "[data]") {
        cleanSystem();

        // Get state keys
        std::string user = getEmulatorUser();

        // Write to the file
        SparseMatrix<double> mat = faasm::randomSparseMatrix(5, 10, 0.4);
        path dummyDir = _setUp();
        SparseMatrixFileSerialiser s(mat);
        s.writeToFile(dummyDir.string());

        const SparseKeys keys = faasm::getSparseKeys(user.c_str());

        // Write to state from files
        _readFileWriteState(user, dummyDir, "vals", keys.valueKey);
        _readFileWriteState(user, dummyDir, "innr", keys.innerKey);
        _readFileWriteState(user, dummyDir, "outr", keys.outerKey);
        _readFileWriteState(user, dummyDir, "size", keys.sizeKey);
        _readFileWriteState(user, dummyDir, "nonz", keys.nonZeroKey);

        // Load matrix from state
        const Map<const SparseMatrix<double>> actual = faasm::readSparseMatrixFromState(user.c_str(), true);
        checkSparseMatrixEquality(mat, actual);
    }
}
