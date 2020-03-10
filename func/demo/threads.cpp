#include <pthread.h>
#include <stdio.h>

void *threadFunc(void *arg) {
    // Increment input variable
    int *intArg = (int *) arg;
    while (++(*intArg) < 200);

    return nullptr;
}

int main() {
    int x = 0, y = 0;

    pthread_t t;
    int ret = pthread_create(&t, NULL, threadFunc, &x);
    if (ret != 0) {
        fprintf(stderr, "Error creating thread (%i)\n", ret);
        return 1;
    }

    // Loop to increment y in main thread
    while (++y < 100);

    if (pthread_join(t, nullptr)) {
        fprintf(stderr, "Error joining thread\n");
        return 1;
    }

    if(y != 100 || x != 200) {
        printf("Thread invocation failed (got x=%i, y=%i)\n", x, y);
        return 1;
    }

    return 0;
}