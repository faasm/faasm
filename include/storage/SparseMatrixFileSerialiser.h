#pragma once

#include <faasm/matrix.h>
#include <faasm/sgd.h>
#include <shared_mutex>

using namespace faasm;

namespace storage {
    class SparseMatrixFileSerialiser : public SparseMatrixSerialiser {
    public:
        explicit SparseMatrixFileSerialiser(const SparseMatrix<double> &matIn);

        void writeToFile(const std::string &directory);

        static SparseMatrix<double> readFromFiles(const std::string &directory);
    };
}
