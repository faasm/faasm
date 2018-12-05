#include <catch/catch.hpp>

#include "utils.h"
#include <data/data.h>

#include <boost/filesystem.hpp>

using namespace data;
using namespace boost::filesystem;

namespace tests {

    TEST_CASE("Check sparse matrix file round trip", "[data]") {
        SparseMatrix<double> mat = faasm::randomSparseMatrix(5, 10, 0.4);

        path dummyDir = "/tmp/faasm-test/";

        if (exists(dummyDir)) {
            remove_all(dummyDir);
        }

        SparseMatrixFileSerialiser s(mat);
        s.writeToFile(dummyDir.string());

        SparseMatrix<double> actual = SparseMatrixFileSerialiser::readFromFiles(dummyDir.string());

        checkSparseMatrixEquality(mat, actual);
    }
}