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

            // Prepare input data for the worker as a string
            std::string args = std::to_string(w) + " " + std::to_string(startIdx) + " " + std::to_string(endIdx) + " " + std::to_string(epoch);
            printf("Chaining sgd_step with args: %s\n", args.c_str());

            auto inputBytes = reinterpret_cast<const uint8_t *>(args.c_str());

            // Call the chained function
            memory->chainFunction("sgd_step", inputBytes, args.size());
        }

        delete[] batchNumbers;

        return 0;
    }
}