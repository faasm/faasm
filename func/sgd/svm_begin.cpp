#include "faasm/faasm.h"
#include "faasm/counter.h"
#include "faasm/sgd.h"

#include <stdio.h>

namespace faasm {
    int exec(FaasmMemory *memory) {
        // Batch size should be small enough that batches don't overwrite each other too often,
        // but large enough that we aren't doing too many function calls.
        // This function allows passing the batch size in as input.
        long inputSize = memory->getInputSize();
        int batchSize;
        if(inputSize == 0) {
            batchSize = REUTERS_N_EXAMPLES/5;
        } else {
            auto inputBuffer = new uint8_t[inputSize];
            memory->getInput(inputBuffer, inputSize);
            char* inputString = reinterpret_cast<char*>(inputBuffer);
            batchSize = std::stoi(inputString);
        }

        // Prepare params
        int epochs = 30;
        SgdParams p = setUpReutersParams(memory, batchSize, epochs);

        // Initialise weights
        Eigen::MatrixXd weights = zeroMatrix(1, p.nWeights);
        writeMatrixToState(memory, WEIGHTS_KEY, weights, p.fullAsync);

        // Zero the losses
        zeroLosses(memory, p);

        // Begin first epoch
        initCounter(memory, EPOCH_COUNT_KEY, p.fullAsync);
        memory->chainFunction("sgd_epoch");

        return 0;
    }
}