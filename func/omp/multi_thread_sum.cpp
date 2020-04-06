#include <omp.h>
#include <faasm/faasm.h>
#include <faasm/array.h>
#include <pthread.h>
#include <faasm/compare.h>

#define N_DEVICE 2
#define N_THREADS 1
#define ARRAY_KEY "multi_thread_sum"
#define ARRAY_SIZE ((N_DEVICE) * (N_THREADS))

void *threadIncrement(void *voidArgs) {
//    auto threadNo = *((int *) voidArgs);
    int threadNo = omp_get_thread_num();

    faasm::AsyncArray<int> distArray(ARRAY_KEY, ARRAY_SIZE);

    // Do the update
    distArray.pullLazy();
    distArray[threadNo] = threadNo;
    distArray.push();

    return nullptr;
}

FAASM_MAIN_FUNC() {
    omp_set_default_device(-N_DEVICE);
    faasm::AsyncArray<int> distArray(ARRAY_KEY, ARRAY_SIZE);
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
