#include <faasm/faasm.h>
#include <faasm/array.h>
#include <pthread.h>
#include <faasm/compare.h>

#define N_THREADS 100
#define ARRAY_KEY "shared_array"

void *threadIncrement(void *voidArgs) {
    auto threadNo = (int *) voidArgs;
    faasm::AsyncArray<int> vals(ARRAY_KEY, N_THREADS);

    // Do the update
    vals.pull();
    vals.set(*threadNo, *threadNo);

    return nullptr;
}

FAASM_MAIN_FUNC() {
    int nThreads = 100;

    faasm::AsyncArray<int> vals(ARRAY_KEY, N_THREADS);
    vals.init();

    pthread_t threads[nThreads];
    int threadArgs[nThreads];
    int expected[nThreads];
    for (int t = 0; t < nThreads; t++) {
        threadArgs[t] = t;
        pthread_create(&threads[t], nullptr, threadIncrement, &threadArgs[t]);

        expected[t] = t;
    }

    for (auto &t : threads) {
        if (pthread_join(t, nullptr)) {
            return 1;
        };
    }

    vals.pull();
    if(!faasm::compareArrays(vals.data(),expected, nThreads)) {
        return 1;
    }

    return 0;
}
