#include <pthread.h>
#include <stdio.h>

/**
 * This function uses a thread that modifies a value on the stack
 * of the main thread, hence will only work when running in local
 * threading mode.
 */

void *threadFunc(void *arg) {
    // Write to the variable passed in
    int *intArg = (int *) arg;
    *intArg = 200;
    return nullptr;
}

int main() {
    int x = 0;

    // Spawn a thread
    pthread_t t;
    int ret = pthread_create(&t, NULL, threadFunc, &x);
    if (ret != 0) {
        fprintf(stderr, "Error creating thread (%i)\n", ret);
        return 1;
    }

    // Join the thread
    if (pthread_join(t, nullptr)) {
        fprintf(stderr, "Error joining thread\n");
        return 1;
    }

    // Check stack variable
    if(x != 200) {
        printf("Thread invocation failed (expected 200 but got %i)\n", x);
        return 1;
    }

    return 0;
}