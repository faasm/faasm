#include "faasm.h"
#include "matrix.h"
#include "counter.h"
#include "random.h"

namespace faasm {
    int nWorkers = 10;

    bool checkWorkers(FaasmMemory *memory) {
        // Check state for each worker
        for (int i = 0; i < nWorkers; i++) {
            char workerKey[10];
            sprintf(workerKey, "worker-%i", i);

            uint8_t count = getCounter(memory, workerKey);
            if (count < 1) {
                return false;
            }
        }

        return true;
    }

    void resetWorkerCounts(FaasmMemory *memory) {
        // Reset for each worker
        for (int i = 0; i < nWorkers; i++) {
            char workerKey[10];
            sprintf(workerKey, "worker-%i", i);
            initCounter(memory, workerKey);
        }
    }

    int exec(FaasmMemory *memory) {
        // See how many attempts we've had
        const char *epochCountKey = "epochCount";
        uint8_t epochCount = getCounter(memory, epochCountKey);

        // Drop out if over max
        uint8_t maxEpochs = 10;
        if (epochCount >= maxEpochs) {
            printf("Workers not finished after %i attempts\n", epochCount);
            return 0;
        }

        // Increment number of attempts
        epochCount++;
        incrementCounter(memory, epochCountKey);

        // Check if workers have finished
        printf("Checking workers on attempt %i\n", epochCount);
        bool workersFinished = checkWorkers(memory);

        // Resubmit if not yet finished
        if (!workersFinished) {
            printf("Workers not finished, on attempt %i\n", epochCount);

            // Recursive call
            uint8_t barrierInput[1] = {0};
            memory->chainFunction("sgd_barrier", barrierInput, 1);
            return 0;
        }

        printf("Workers finished\n");

        // Reset all worker counts
        resetWorkerCounts(memory);

        return 0;
    }
}