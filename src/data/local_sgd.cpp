#include "data.h"

#include <util/util.h>

#include "faasm/matrix.h"
#include "faasm/sgd.h"
#include "faasm/time.h"

namespace data {
    static std::mutex lossMutex;
    static std::vector<double> losses;
    static std::vector<double> lossTimestamps;

    void clear() {
        losses.clear();
        lossTimestamps.clear();
    }

    std::vector<double> getLosses() {
        return losses;
    }

    std::vector<double> getLossTimestamps() {
        return lossTimestamps;
    }

    void runPool(const SgdParams &params, int epoch) {
        // Spawn a thread for each batch
        std::thread threads[params.nBatches];
        for (int b = 0; b < params.nBatches; b++) {
            threads[b] = std::thread(data::run, epoch, b);
        }

        // Wait for threads to finish
        for(auto& t : threads) {
            if(!t.joinable()) {
                throw std::runtime_error("Thread not joinable");
            }
            t.join();
        }
    }

    void run(int epoch, int batchNumber) {
        FaasmMemory memory;

        SgdParams params = readParamsFromState(&memory, PARAMS_KEY, true);

        int batchSize = params.nTrain / params.nBatches;
        int startIdx = batchNumber * batchSize;
        int endIdx = startIdx + batchSize;

        SparseMatrix<double> inputs = readSparseMatrixColumnsFromState(&memory, INPUTS_KEY, startIdx, endIdx, true);
        MatrixXd outputs = readMatrixColumnsFromState(&memory, OUTPUTS_KEY, params.nTrain, startIdx, endIdx, 1, true);

        // Perform the update
        MatrixXd actual = hingeLossWeightUpdate(&memory, params, epoch, inputs, outputs);

        // Calculate the loss and add to the list
        double loss = calculateHingeError(actual, outputs);
        long ts = faasm::getMillisSinceEpoch();
        std::unique_lock<std::mutex> lock(lossMutex);
        losses.push_back(loss);
        lossTimestamps.push_back(ts);
    }
}
