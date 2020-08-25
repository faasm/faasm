#include "SparseMatrixFileSerialiser.h"

#include <faabric/util/files.h>
#include <faabric/util/logging.h>

#include <faasm/matrix.h>

#include <boost/filesystem.hpp>

using namespace boost::filesystem;

namespace storage {
    struct SparseFiles {
        path valuesPath;
        path innersPath;
        path outerPath;
        path sizePath;
        path nonZeroPath;
    };

    // These filenames should match with the SparseKeys structs to make
    // transferring more simple
    SparseFiles getSparseFiles(const path &basePath) {
        SparseFiles files{};

        files.valuesPath = basePath;
        files.valuesPath.append("vals");

        files.innersPath = basePath;
        files.innersPath.append("innr");

        files.outerPath = basePath;
        files.outerPath.append("outr");

        files.sizePath = basePath;
        files.sizePath.append("size");

        files.nonZeroPath = basePath;
        files.nonZeroPath.append("nonz");

        return files;
    }

    void _doWriteToFile(const path &filePath, const uint8_t *bytesPtr, size_t bytesLen) {
        const std::shared_ptr<spdlog::logger> &logger = faabric::util::getLogger();
        const std::vector<uint8_t> bytes = std::vector<uint8_t>(bytesPtr, bytesPtr + bytesLen);

        logger->info("Writing {} bytes to {}", bytes.size(), filePath.c_str());

        faabric::util::writeBytesToFile(filePath.string(), bytes);
    }

    SparseMatrixFileSerialiser::SparseMatrixFileSerialiser(const SparseMatrix<double> &matIn) :
            SparseMatrixSerialiser(matIn) {

    }

    void SparseMatrixFileSerialiser::writeToFile(const std::string &directory) {
        path dir(directory);
        SparseFiles files = getSparseFiles(dir);

        boost::filesystem::create_directories(dir);

        _doWriteToFile(files.valuesPath, valueBytes, nValueBytes);
        _doWriteToFile(files.innersPath, innerBytes, nInnerBytes);
        _doWriteToFile(files.outerPath, outerBytes, nOuterBytes);
        _doWriteToFile(files.sizePath, sizeBytes, nSizeBytes);
        _doWriteToFile(files.nonZeroPath, nonZeroBytes, nNonZeroBytes);
    }

    SparseMatrix<double> SparseMatrixFileSerialiser::readFromFiles(const std::string &directory) {
        path dir(directory);
        SparseFiles files = getSparseFiles(dir);

        std::vector<uint8_t> sizeBytes = faabric::util::readFileToBytes(files.sizePath.string());
        auto sizes = reinterpret_cast<faasm::SparseSizes *>(sizeBytes.data());

        std::vector<uint8_t> valueBytes = faabric::util::readFileToBytes(files.valuesPath.string());
        std::vector<uint8_t> innerBytes = faabric::util::readFileToBytes(files.innersPath.string());
        std::vector<uint8_t> outerBytes = faabric::util::readFileToBytes(files.outerPath.string());

        return SparseMatrixSerialiser::readFromBytes(
                *sizes,
                outerBytes.data(),
                innerBytes.data(),
                valueBytes.data()
        );
    }
}


