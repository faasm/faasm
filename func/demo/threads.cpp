#include <pthread.h>
#include <stdio.h>

void *inc_x(void *x_void_ptr) {
    int *x_ptr = (int *) x_void_ptr;
    while (++(*x_ptr) < 200);

    printf("x increment finished\n");

    return NULL;
}

int main() {
    int x = 0, y = 0;
    printf("x: %d, y: %d\n", x, y);

    pthread_t inc_x_thread;

    int ret = pthread_create(&inc_x_thread, NULL, inc_x, &x);
    if (ret != 0) {
        fprintf(stderr, "Error creating thread (%i)\n", ret);
        return 1;
    }

    // Loop to increment y in first thread
    while (++y < 100);
    printf("y increment finished\n");

    if (pthread_join(inc_x_thread, NULL)) {
        fprintf(stderr, "Error joining thread\n");
        return 2;
    }

    if(y != 100 || x != 200) {
        printf("Thread invocation failed (got x=%i, y=%i)\n", x, y);
        return 1;
    }

    printf("x: %d, y: %d\n", x, y);
    return 0;
}