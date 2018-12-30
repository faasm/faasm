#include <iostream>
#include <fstream>

#include <data/data.h>
#include <emulator/emulator.h>
#include <util/util.h>
#include <infra/infra.h>
#include <state/state.h>

#include <faasm/matrix.h>
#include <faasm/sgd.h>
#include <faasm/counter.h>
#include <faasm/time.h>

using namespace faasm;

int main() {
    util::initLogging();
    state::getGlobalState().forceClearAll();
    const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

    // Override emulator user
    setEmulatorUser("sgd");

    FaasmMemory memory;
    int nBatches = 4;
    int batchSize = (nBatches + REUTERS_N_EXAMPLES - 1) / REUTERS_N_EXAMPLES;

    int epochs = 30;
    SgdParams p = setUpReutersParams(&memory, batchSize, epochs);

    logger->info("Running SVM with {} threads (batch size {})", p.nBatches, p.batchSize);

    // Initialise weights to zero
    Eigen::MatrixXd weights = faasm::zeroMatrix(1, p.nWeights);
    writeMatrixToState(&memory, WEIGHTS_KEY, weights, REUTERS_FULL_ASYNC);

    // Clear out existing state
    faasm::zeroLosses(&memory, p);

    // Run each epoch
    std::vector<std::pair<double, double>> losses;
    double startTs = faasm::getSecondsSinceEpoch();
    for (int epoch = 0; epoch < p.nEpochs; epoch++) {
        logger->info("Epoch {} start", epoch);

        faasm::zeroErrors(&memory, p);
        faasm::zeroFinished(&memory, p);

        data::runPool(p, epoch);

        // Work out error
        double rmse = data::getRMSE(p);
        double thisTs = faasm::getSecondsSinceEpoch() - startTs;

        losses.emplace_back(std::pair<double, double>(thisTs, rmse));

        logger->info("Epoch {} end   - time {:04.2f}s - RMSE {:06.4f}", epoch, thisTs, rmse);

        // Decay learning rate
        p.learningRate = p.learningRate * p.learningDecay;
        faasm::writeParamsToState(&memory, PARAMS_KEY, p);
    }

    std::ofstream resultFile;
    resultFile.open("measurement/losses.txt");
    for (auto loss : losses) {
        resultFile << loss.first << " " << loss.second << std::endl;
    }
    resultFile.close();
}
