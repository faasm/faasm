#include "data.h"

#include <util/util.h>

#include "faasm/matrix.h"
#include "faasm/sgd.h"
#include "faasm/time.h"

namespace data {
    static std::mutex errorsMutex;
    static std::vector<double> errors;

    void clear() {
        errors.clear();
    }

    double getRMSE(const SgdParams &p) {
        double totalError = 0;
        for(auto e : errors) {
            totalError += e;
        }

        return std::sqrt(totalError) / std::sqrt(p.nTrain);
    }

    std::vector<double> getErrors() {
        return errors;
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

        int startIdx = batchNumber * params.batchSize;
        int endIdx = std::min(startIdx + params.batchSize, params.nTrain - 1);

        SparseMatrix<double> inputs = readSparseMatrixColumnsFromState(&memory, INPUTS_KEY, startIdx, endIdx, true);
        MatrixXd outputs = readMatrixColumnsFromState(&memory, OUTPUTS_KEY, params.nTrain, startIdx, endIdx, 1, true);

        // Perform the update
        MatrixXd actual = hingeLossWeightUpdate(&memory, params, epoch, inputs, outputs);

        // Calculate the error and add to the list
        double error = calculateHingeError(actual, outputs);

        std::unique_lock<std::mutex> lock(errorsMutex);
        errors.push_back(error);
    }
}
