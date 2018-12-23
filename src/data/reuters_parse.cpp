#include "data/data.h"

#include <string>
#include <iostream>
#include <boost/filesystem.hpp>
#include <util/util.h>

#include <faasm/sgd.h>


using namespace boost::filesystem;

void parseReutersData(const path &downloadDir, const path &outputDir) {
    const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

    std::string targetCategory = "CCAT";

    std::vector<std::string> files = {
            "Index_EN-EN",
            "Index_FR-EN",
            "Index_GR-EN",
            "Index_IT-EN",
            "Index_SP-EN",
    };

    int exampleCount = 0;
    int maxFeature = 0;

    std::vector<double> categories;
    std::vector<Eigen::Triplet<double>> triplets;

    std::vector<int> featureCounts(REUTERS_N_FEATURES);

    for (const auto &f : files) {
        path thisFile = downloadDir;
        thisFile.append(f);
        logger->info("Reading from {} ", thisFile.c_str());

        std::ifstream input(thisFile.c_str());

        std::string line;
        while (getline(input, line)) {
            // Split up the line
            const std::vector<std::string> lineTokens = util::tokeniseString(line, ' ');
            std::string cat = lineTokens[0];

            if (cat == targetCategory) {
                categories.push_back(1);
            } else {
                categories.push_back(-1);
            }

            for (int i = 1; i < lineTokens.size(); i++) {
                // Ignore empty token
                std::basic_string<char> thisToken = lineTokens[i];
                if (util::isAllWhitespace(thisToken)) {
                    continue;
                }

                // Split up index:value part
                const std::vector<std::string> valueTokens = util::tokeniseString(thisToken, ':');
                int feature = std::stoi(valueTokens[0]);
                double weight = std::stof(valueTokens[1]);

                // Add to matrix.
                // NOTE: the file is 1-based so we knock one off the feature value.
                // With triplets, eigen is zero-based (although when indexing an array it's 1-based)
                triplets.emplace_back(Eigen::Triplet<double>(feature - 1, exampleCount, weight));

                // Record an occurrence of this feature
                featureCounts.at(feature - 1)++;

                maxFeature = std::max(maxFeature, feature);
            };

            // Move onto next example
            exampleCount++;
        }

        // Close off the file
        input.close();
    }

    if (maxFeature != REUTERS_N_FEATURES) {
        logger->error("Expected {} features but got {}", REUTERS_N_FEATURES, maxFeature);
    }

    logger->info("Totals: {} features and {} examples", maxFeature, exampleCount);

    // Build the sparse matrix (row for each feature, col for each example)
    Eigen::SparseMatrix<double> mat(maxFeature, exampleCount);
    mat.setFromTriplets(triplets.begin(), triplets.end());

    // Write matrix to files
    data::SparseMatrixFileSerialiser s(mat);
    s.writeToFile(outputDir.string());

    // Write categories to file
    long nCatBytes = categories.size() * sizeof(double);
    auto catPtr = reinterpret_cast<uint8_t *>(categories.data());

    path catFile = outputDir;
    catFile.append("outputs");

    logger->info("Writing {} bytes to {}", nCatBytes, catFile.c_str());
    util::writeBytesToFile(catFile.string(), std::vector<uint8_t>(catPtr, catPtr + nCatBytes));

    // Write feature counts to file
    long nFeatureCountBytes = featureCounts.size() * sizeof(int);
    auto featureCountsPtr = reinterpret_cast<uint8_t *>(featureCounts.data());

    path featureCountsFile = outputDir;
    featureCountsFile.append("feature_counts");

    logger->info("Writing {} bytes to {}", nFeatureCountBytes, featureCountsFile.c_str());
    util::writeBytesToFile(featureCountsFile.string(), std::vector<uint8_t>(featureCountsPtr,
                                                                            featureCountsPtr + nFeatureCountBytes));
}

int main() {
    util::initLogging();

    path faasmDir(util::getEnvVar("HOME", ""));
    faasmDir.append("faasm");

    path downloadDir = faasmDir;
    downloadDir.append("rcv1rcv2aminigoutte/EN");

    path outputDir = faasmDir;
    outputDir.append("data/reuters");

    if (exists(outputDir)) {
        remove_all(outputDir);
    }

    create_directories(outputDir);

    parseReutersData(downloadDir, outputDir);

    return 0;
}
