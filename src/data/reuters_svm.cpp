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

    FaasmMemory memory;
    int batchSize = 5587;
    bool fullAsync = true;
    SgdParams p = setUpReutersParams(&memory, batchSize, fullAsync);

    const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

    // Initialise weights
    Eigen::MatrixXd weights = faasm::randomDenseMatrix(1, p.nWeights);
    writeMatrixToState(&memory, WEIGHTS_KEY, weights, true);

    // Run each epoch
    for (int epoch = 0; epoch < p.nEpochs; epoch++) {
        data::LocalWorker::runPool(p, epoch);

        // Work out error
        double totalError = 0;
        std::vector<double> losses = data::LocalWorker::getLosses();
        std::vector<double> lossTimestamps = data::LocalWorker::getLossTimestamps();

        for (auto l : losses) {
            totalError += l;
        }

        double rmse = sqrt(totalError / p.nTrain);
        logger->info("Epoch {} - RMSE {}", epoch, rmse);

        data::LocalWorker::clear();
    }
}
