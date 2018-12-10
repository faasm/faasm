#include <catch/catch.hpp>

#include "utils.h"

#include <data/data.h>
#include <util/util.h>
#include <infra/infra.h>
#include <faasm/matrix.h>
#include <faasm/sgd.h>

using namespace faasm;

namespace tests {

    TEST_CASE("Placeholder", "[data]") {
        REQUIRE(true);
    }

//    TEST_CASE("Test SVM on reuters dataset", "[data]") {
//        infra::Redis cli;
//        cli.flushAll();
//
//        // Read in input data
//        std::string dataDir = "/tmp/reuters_out";
//        SparseMatrix<double> inputs = data::SparseMatrixFileSerialiser::readFromFiles(dataDir);
//
//        // Read category data
//        std::string outputFile = "/tmp/reuters_out/outputs";
//        std::vector<uint8_t> outputBytes = util::readFileToBytes(outputFile);
//
//        // Read in outputs
//        auto outputPtr = reinterpret_cast<char *>(outputBytes.data());
//        MatrixXd outputs(1, REUTERS_N_EXAMPLES);
//        for (int i = 0; i < REUTERS_N_EXAMPLES; i++) {
//            outputs(0, i) = outputPtr[i];
//        }
//
//        // Initialise the memory
//        FaasmMemory mem;
//
//        // Set up params
//        faasm::SgdParams p;
//        p.lossType = faasm::HINGE;
//        p.nEpochs = 10;
//        p.nWeights = REUTERS_N_FEATURES;
//        p.learningRate = REUTERS_LEARNING_RATE;
//
//        // Use small subset of data
//        p.nTrain = 150;
//
//        // Do each epoch in a single loop
//        p.nBatches = 1;
//
//        // Initialise weights
//        Eigen::MatrixXd weights = faasm::randomDenseMatrix(1, p.nWeights);
//        writeMatrixToState(&mem, WEIGHTS_KEY, weights);
//
//        MatrixXd initialPrediction = weights * inputs;
//        double startingLoss = faasm::calculateHingeError(initialPrediction, outputs);
//        REQUIRE(startingLoss > 0);
//
//        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
//
//        // Run multiple updates
//        double finalLoss = 0;
//        for (int e = 0; e < p.nEpochs; e++) {
//            logger->info("Test SVM step {}", e);
//            finalLoss = doSgdStep(&mem, p, e, inputs, outputs);
//        }
//
//        REQUIRE(finalLoss < startingLoss);
//    }
}