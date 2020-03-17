#include <faasm/faasm.h>
#include <faasm/array.h>
#include <pthread.h>
#include <faasm/compare.h>

#define N_THREADS 4
#define ARRAY_KEY "shared_array"

void *threadIncrement(void *voidArgs) {
    auto threadNo = *((int *) voidArgs);
    faasm::AsyncArray<int> distArray(ARRAY_KEY, N_THREADS);

    // Do the update
    distArray.pullLazy();
    distArray[threadNo] = threadNo;
    distArray.push();

    return nullptr;
}

FAASM_MAIN_FUNC() {
    faasm::AsyncArray<int> distArray(ARRAY_KEY, N_THREADS);
    distArray.zero();

    // Set up arguments
    pthread_t threads[N_THREADS];
    int threadArgs[N_THREADS];
    int expected[N_THREADS];
    for (int t = 0; t < N_THREADS; t++) {
        threadArgs[t] = t;
        expected[t] = t;
    }

    // Spawn threads
    for (int t = 0; t < N_THREADS; t++) {
        pthread_create(&threads[t], nullptr, threadIncrement, &threadArgs[t]);
    }

    // Join threads
    for (auto &t : threads) {
        if (pthread_join(t, nullptr)) {
            return 1;
        };
    }

    // Check
    distArray.pull();
    if (!faasm::compareArrays(distArray.data(), expected, N_THREADS)) {
        return 1;
    }

    return 0;
}
