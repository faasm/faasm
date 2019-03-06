#include "faasm/faasm.h"
#include "faasm/counter.h"
#include "faasm/matrix.h"
#include "faasm/sgd.h"

namespace faasm {
    int exec(FaasmMemory *memory) {
        bool fullAsync = getEnvFullAsync();

        size_t nBytes = sizeof(int) * 4;
        auto inputBuffer = new uint8_t[nBytes];
        memory->getInput(inputBuffer, nBytes);

        const int *inputParams = reinterpret_cast<const int *>(inputBuffer);

        int batchNumber = inputParams[0];
        int startIdx = inputParams[1];
        int endIdx = inputParams[2];
        int epoch = inputParams[3];

        // Load params
        SgdParams sgdParams = readParamsFromState(memory, PARAMS_KEY, fullAsync);

        // Perform updates
        if (sgdParams.lossType == HINGE) {
            hingeLossWeightUpdate(memory, sgdParams, epoch, batchNumber, startIdx, endIdx);
        } else {
            leastSquaresWeightUpdate(memory, sgdParams, batchNumber,startIdx, endIdx);
        }

        // Flag that this worker has finished
        writeFinishedFlag(memory, sgdParams, batchNumber);

        // If this is the last, dispatch the barrier (will have finished by now or will do soon)
        if (batchNumber == sgdParams.nBatches - 1) {
            memory->chainFunction("sgd_barrier");
        }

        return 0;
    }
}