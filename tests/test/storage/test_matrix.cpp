#include <catch/catch.hpp>

#include "utils.h"

#include <storage/SparseMatrixFileSerialiser.h>

#include <boost/filesystem.hpp>
#include <util/state.h>
#include <util/files.h>
#include <emulator/emulator.h>

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

    void _readFileWriteState(path dummyDir, const std::string &matrixPart, const std::string &key) {
        // Read in from file
        path filePath = dummyDir;
        filePath.append(matrixPart);
        const std::vector<uint8_t> fileBytes = util::readFileToBytes(filePath.string());

        // Write to state
        const std::string user = getEmulatorUser();
        const std::shared_ptr<state::StateKeyValue> kv = state::getGlobalState().getKV(user, key, fileBytes.size());
        kv->set(fileBytes.data());
        kv->pushFull();
    }

    TEST_CASE("Check sparse matrix file round trip", "[data]") {
        SparseMatrix<double> mat = faasm::randomSparseMatrix(5, 10, 0.4);

        path dummyDir = _setUp();
        storage::SparseMatrixFileSerialiser s(mat);
        s.writeToFile(dummyDir.string());

        // Load sizes and check they still match
        path sizePath = dummyDir;
        sizePath.append("size");
        const std::vector<uint8_t> &sizeBytes = util::readFileToBytes(sizePath.string());
        auto sizes = reinterpret_cast<const SparseSizes *>(sizeBytes.data());

        REQUIRE(sizes->rows == 5);
        REQUIRE(sizes->cols == 10);

        // Read all files in
        SparseMatrix<double> actual = storage::SparseMatrixFileSerialiser::readFromFiles(dummyDir.string());
        checkSparseMatrixEquality(mat, actual);
    }

    TEST_CASE("Check sparse matrix file and state round trip", "[data]") {
        cleanSystem();

        std::string key = "sparse_mat_round_trip";

        // Write to the file
        SparseMatrix<double> mat = faasm::randomSparseMatrix(5, 10, 0.4);
        path dummyDir = _setUp();
        storage::SparseMatrixFileSerialiser s(mat);
        s.writeToFile(dummyDir.string());

        // Write to state from files
        const SparseKeys keys = faasm::getSparseKeys(key.c_str());
        _readFileWriteState(dummyDir, "vals", keys.valueKey);
        _readFileWriteState(dummyDir, "innr", keys.innerKey);
        _readFileWriteState(dummyDir, "outr", keys.outerKey);
        _readFileWriteState(dummyDir, "size", keys.sizeKey);
        _readFileWriteState(dummyDir, "nonz", keys.nonZeroKey);

        // Load matrix from state
        const Map<const SparseMatrix<double>> actual = faasm::readSparseMatrixFromState(key.c_str(), true);
        checkSparseMatrixEquality(mat, actual);
    }
}
