#include <data/data.h>
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

    FaasmMemory memory;
    // int batchSize = 69000;
    int batchSize = REUTERS_N_EXAMPLES;
    int epochs = 30;
    SgdParams p = setUpReutersParams(&memory, batchSize, epochs, true);

    logger->info("Running SVM with {} threads", p.nBatches);

    // Initialise weights to zero
    Eigen::MatrixXd weights = faasm::zeroMatrix(1, p.nWeights);
    writeMatrixToState(&memory, WEIGHTS_KEY, weights, true);

    // Run each epoch
    long startTs = faasm::getMillisSinceEpoch();
    for (int epoch = 0; epoch < p.nEpochs; epoch++) {
        data::clear();

        data::runPool(p, epoch);

        // Work out error
        double rmse = data::getRMSE(p);

        double seconds = ((double)(faasm::getMillisSinceEpoch() - startTs)) / 1000;
        logger->info("Epoch {} - time {:04.2f}s - RMSE {:06.4f}", epoch, seconds, rmse);

        // Decay learning rate
        p.learningRate = p.learningRate * p.learningDecay;
        faasm::writeParamsToState(&memory, PARAMS_KEY, p);
    }
}
