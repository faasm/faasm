#include "faasm/faasm.h"
#include "faasm/sgd.h"

namespace faasm {
    int exec(FaasmMemory *memory) {
        int startIdx = 0;
        int endIdx = 10;

        // Load the data
        MatrixXd weights = readMatrixFromState(memory, WEIGHTS_KEY, 1, REUTERS_N_FEATURES);
        SparseMatrix<double> inputs = readSparseMatrixColumnsFromState(memory, INPUTS_KEY, startIdx, endIdx);
        MatrixXd outputs = readMatrixColumnsFromState(memory, OUTPUTS_KEY, startIdx, endIdx, 1);

        printf("W: %liX%li\n", weights.rows(), weights.cols());
        printf("I: %liX%li\n", inputs.rows(), inputs.cols());
        printf("O: %liX%li\n", outputs.rows(), outputs.cols());

        for (int i = 0; i < 10; i++) {
            printf("W: %.2f  I: %.2f  O: %.2f\n", weights.coeff(0, i), inputs.coeff(i, 0), outputs.coeff(0, i));
        }

        return 0;
    }
}