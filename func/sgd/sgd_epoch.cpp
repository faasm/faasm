#include "faasm/faasm.h"
#include "faasm/matrix.h"
#include "faasm/counter.h"
#include "faasm/sgd.h"

namespace faasm {
    int exec(FaasmMemory *memory) {
        // Load params
        bool fullAsync = getEnvFullAsync();
        SgdParams p = readParamsFromState(memory, PARAMS_KEY, fullAsync);

        // Set per-epoch memory to zero
        faasm::zeroErrors(memory, p);
        faasm::zeroFinished(memory, p);

        // Reset the barrier counter
        faasm::initCounter(memory, BARRIER_COUNT_KEY, fullAsync);

        // Get the epoch count
        int epoch = faasm::getCounter(memory, EPOCH_COUNT_KEY, p.fullAsync);

        // Shuffle start indices for each batch
        int *batchNumbers = faasm::randomIntRange(p.nBatches);

        // Chain new calls to perform the work
        for (int w = 0; w < p.nBatches; w++) {
            int startIdx = batchNumbers[w] * p.batchSize;

            // Make sure we don't overshoot
            int endIdx = std::min(startIdx + p.batchSize, p.nTrain - 1);

            // Prepare input data for the worker
            int inputData[4] = {w, startIdx, endIdx, epoch};
            auto inputBytes = reinterpret_cast<uint8_t *>(&inputData[0]);
            int nBytes = 4 * sizeof(int);

            // Call the chained function
            memory->chainFunction("sgd_step", inputBytes, nBytes);
        }

        delete[] batchNumbers;

        return 0;
    }
}