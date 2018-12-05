#pragma once

#include <faasm/matrix.h>

namespace data {

    class SparseMatrixFileSerialiser : public faasm::SparseMatrixSerialiser {
    public:
        using SparseMatrixSerialiser::SparseMatrixSerialiser;

        void writeToFile(const std::string &directory);
    };

}
