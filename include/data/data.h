#pragma once

#include <faasm/matrix.h>
#include <faasm/sgd.h>
#include <shared_mutex>

using namespace faasm;

namespace data {
    class SparseMatrixFileSerialiser : public SparseMatrixSerialiser {
    public:
        using SparseMatrixSerialiser::SparseMatrixSerialiser;

        void writeToFile(const std::string &directory);

        static SparseMatrix<double> readFromFiles(const std::string &directory);
    };

    class LocalWorker {
    public:
        LocalWorker();

        static void runPool(const SgdParams &params, int epoch);

        static void clear();

        static std::vector<double> getLosses();

        static std::vector<double> getLossTimestamps();

        static void run(int epoch, int batchNumber);
    };
}
