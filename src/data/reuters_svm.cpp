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
    writeMatrixToState(&memory, WEIGHTS_KEY, weights);

    // Zero the losses
    zeroLosses(&memory, p);

    // Get full input/ output data in memory
    const SparseMatrix<double> inputs = readSparseMatrixFromState(&memory, INPUTS_KEY, false);
    const MatrixXd outputs = readMatrixFromState(&memory, OUTPUTS_KEY, 1, p.nTrain, false);

    // Run each epoch
    for (int epoch = 0; epoch < p.nEpochs; epoch++) {
        // Spawn a thread for each batch
        std::vector<std::thread> threads(p.nBatches);
        for (int b = 0; b < p.nBatches; b++) {
            threads.emplace_back(std::thread(
                    data::LocalWorker::run, p, epoch, b, inputs, outputs
            ));
        }

        // Wait for all threads
        for (auto &t : threads) {
            if(t.joinable()) {
                t.join();
            }
            else {
                logger->error("Thread not joinable");
            }
        }

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
