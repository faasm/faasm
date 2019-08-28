#include <unistd.h>
#include <system_error>
#include <iostream>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SLEEP_SECONDS 60
#define N_THREADS 35000
#define STACK_SIZE 1024


pthread_t pthreads[N_THREADS];
static int threadCount = 0;

void *thread_inner(void *p) {
    sleep(SLEEP_SECONDS);

    pthread_exit(0);
}

void segfaultHandler(int signal, siginfo_t *si, void *arg) {
    printf("Segfaulted at thread %i\n", threadCount);
    exit(0);
}

bool _doThreadSpawn(int i) {
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, STACK_SIZE);

    try {
        pthread_create(&pthreads[i], &attr, &thread_inner, NULL);
    } catch (std::system_error &e) {
        // std::cerr << "What: " << e.what() << "  Code: " << e.code() << std::endl;
        return false;
    }

    return true;
}

int main() {
    for (int i = 0; i < N_THREADS; i++) {
        struct sigaction sa;

        memset(&sa, 0, sizeof(struct sigaction));
        sigemptyset(&sa.sa_mask);
        sa.sa_sigaction = segfaultHandler;
        sa.sa_flags   = SA_SIGINFO;

        sigaction(SIGSEGV, &sa, NULL);

        _doThreadSpawn(i);
        threadCount++;
    }

    for (int i = 0; i < N_THREADS; i++) {
        pthread_join(pthreads[i], NULL);
    }

    printf("Finished successfully with %i threads\n", threadCount);
}
