#include "faasm/faasm.h"
#include "faasm/matrix.h"
#include "faasm/counter.h"
#include "faasm/sgd.h"

namespace faasm {
    int exec(FaasmMemory *memory) {
        // Load params
        SgdParams p = readParamsFromState(memory, PARAMS_KEY);

        // Set per-epoch memory to zero
        faasm::zeroErrors(memory, p);
        faasm::zeroFinished(memory, p);

        // Get the epoch count
        int epoch = faasm::getCounter(memory, EPOCH_COUNT_KEY);

        // Shuffle start indices for each batch
        int* batchStartIndices = faasm::randomIntRange(p.nBatches);

        // Chain new calls to perform the work
        int batchSize = p.nTrain / p.nBatches;
        for (int w = 0; w < p.nBatches; w++) {
            int startIdx = batchStartIndices[w];
            int endIdx = startIdx + batchSize;

            // Prepare input data for the worker
            int inputData[4] = {w, startIdx, endIdx, epoch};
            auto inputBytes = reinterpret_cast<uint8_t *>(&inputData[0]);
            int nBytes = 4 * sizeof(int);

            // Call the chained function
            memory->chainFunction("sgd_step", inputBytes, nBytes);
        }

        delete[] batchStartIndices;

        return 0;
    }
}