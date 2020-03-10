#include <faasm/faasm.h>
#include <pthread.h>

faasm::SharedArray<int> vals;

void *threadIncrement(void *voidArgs) {
    auto threadNo = (int *)voidArgs;

    // Do the update
    vals[threadNo] = threadNo;
}

FAASM_MAIN_FUNC() {
    int nThreads = 100;

    pthread_t threads[nThreads];
    int threadArgs[nThreads];
    for(int t = 0; t < nThreads; t++) {
        threadArgs[t] = t;
        pthread_create(&threads[t], nullptr, threadIncrement, &threadArgs[t]);
    }

    for(auto &t : threads) {
        if(pthread_join(t, nullptr)) {
            return 1;
        };
    }

    return 0;
}
