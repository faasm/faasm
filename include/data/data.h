#pragma once

#include <faasm/matrix.h>

namespace data {

    class SparseMatrixFileSerialiser : public faasm::SparseMatrixSerialiser {
    public:
        using SparseMatrixSerialiser::SparseMatrixSerialiser;

        void writeToFile(const std::string &directory);

        static SparseMatrix<double> readFromFiles(const std::string &directory);
    };
}
