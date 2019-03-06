#include "data.h"

#include <util/logging.h>

#include "faasm/matrix.h"
#include "faasm/sgd.h"
#include "faasm/time.h"

namespace data {
    double getRMSE(const SgdParams &p) {
        FaasmMemory memory;
        double rmse = faasm::readRootMeanSquaredError(&memory, p);
        return rmse;
    }

    void runPool(const SgdParams &params, int epoch) {
        // Spawn a thread for each batch
        std::thread threads[params.nBatches];
        for (int b = 0; b < params.nBatches; b++) {
            threads[b] = std::thread(data::run, epoch, b);
        }

        // Wait for threads to finish
        for (auto &t : threads) {
            if (!t.joinable()) {
                throw std::runtime_error("Thread not joinable");
            }
            t.join();
        }
    }

    void run(int epoch, int batchNumber) {
        FaasmMemory memory;
        bool fullAsync = getEnvFullAsync();
        SgdParams params = readParamsFromState(&memory, PARAMS_KEY, fullAsync);

        int startIdx = batchNumber * params.batchSize;
        int endIdx = std::min(startIdx + params.batchSize, params.nTrain - 1);

        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->info("Batch {} ({} - {}, {} examples)", batchNumber, startIdx, endIdx, endIdx - startIdx);

        // Perform the update
        hingeLossWeightUpdate(&memory, params, epoch, batchNumber, startIdx, endIdx);
    }
}
