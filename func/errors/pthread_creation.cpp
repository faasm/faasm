#include "faasm/faasm.h"
#include <stdio.h>
#include <pthread.h>

void *print_message(void *ptr) {
    printf("Threading\n");

    pthread_exit(0);
}

FAASM_MAIN_FUNC() {
    pthread_t t1;

    pthread_create(&t1, NULL, &print_message, NULL);
    printf("Hello\n");

    pthread_join(t1, NULL);
    return 0;
}