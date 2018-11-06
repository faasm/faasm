#include "faasm/faasm.h"
#include "faasm/matrix.h"

#include "svm_constants.h"

namespace faasm {
    int exec(FaasmMemory *memory) {
        const uint8_t *input = memory->getInput();
        const int *inputParams = reinterpret_cast<const int *>(input);

        int workerIdx = inputParams[0];
        int startIdx = inputParams[1];
        int endIdx = inputParams[2];

        MatrixXd weights = readMatrixFromState(memory, WEIGHTS_KEY, 1, N_WEIGHTS);
        MatrixXd inputs = readMatrixColumnsFromState(memory, INPUTS_KEY, startIdx, endIdx, N_WEIGHTS);
        MatrixXd outputs = readMatrixColumnsFromState(memory, OUTPUTS_KEY, startIdx, endIdx, 1);

        MatrixXd predictions = outputs * (weights * inputs);

        for (int i = 0; i < N_WEIGHTS; i++) {
            double prediction = predictions(0, i);

            // Use hinge loss function
            if (prediction < 1) {

            }
        }


        return 0;
    }
}