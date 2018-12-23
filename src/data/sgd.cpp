#include "data.h"

#include <state/state.h>

#include "faasm/matrix.h"
#include "faasm/sgd.h"
#include "faasm/time.h"

namespace data {
    static std::shared_mutex lossMutex;
    static std::vector<double> losses;
    static std::vector<double> lossTimestamps;

    LocalWorker::LocalWorker() = default;

    void LocalWorker::clear() {
        losses.clear();
        lossTimestamps.clear();
    }

    std::vector<double> LocalWorker::getLosses() {
        return losses;
    }

    std::vector<double> LocalWorker::getLossTimestamps() {
        return lossTimestamps;
    }

    void LocalWorker::run(const SgdParams &params, int epoch, int batchNumber,
             const SparseMatrix<double> &inputs, const MatrixXd &outputs) {

        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->info("Running batch {}", batchNumber);

        FaasmMemory mem;
        double batchLoss = data::doSgdStep(&mem, params, epoch, batchNumber, inputs, outputs);

        long ts = faasm::getMillisSinceEpoch();

        state::FullLock lock(lossMutex);
        losses.push_back(batchLoss);
        lossTimestamps.push_back(ts);
    }

    double doSgdStep(FaasmMemory *mem, const SgdParams &params, int epoch, int batchNumber, const SparseMatrix<double> &inputs,
            const MatrixXd &outputs) {

        int batchSize = params.nTrain / params.nBatches;
        for (int b = 0; b < params.nBatches; b++) {
            int startCol = batchNumber * batchSize;

            SparseMatrix<double> inputBatch = inputs.block(0, startCol, params.nWeights, batchSize);
            MatrixXd outputBatch = outputs.block(0, startCol, 1, batchSize);

            // Perform the update
            MatrixXd actual;
            if (params.lossType == RMSE) {
                actual = leastSquaresWeightUpdate(mem, params, inputBatch, outputBatch);

                // Persist error for these examples
                writeSquaredError(mem, params, b, outputBatch, actual);
            } else if (params.lossType == HINGE) {
                actual = hingeLossWeightUpdate(mem, params, epoch, inputBatch, outputBatch);

                // Persist error
                writeHingeError(mem, params, b, outputBatch, actual);
            }

            // Flag that this worker has finished
            writeFinishedFlag(mem, params, b);
        }

        // Get the loss
        double loss = faasm::readRootMeanSquaredError(mem, params);

        return loss;
    }

}
