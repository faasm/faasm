#include "faasm/faasm.h"

#include <time.h>
#include <stdio.h>
#include <unistd.h>

FAASM_MAIN_FUNC() {
    // Time with direct syscall
    timespec tsA{};
    clock_gettime(CLOCK_REALTIME, &tsA);

    // Time with clock
    clock_t clockStart = clock();

    usleep(10);
    printf("Doing something...\n");

    // End with direct syscall
    timespec tsB{};
    clock_gettime(CLOCK_REALTIME, &tsB);

    // End with clock
    clock_t clockEnd = clock();

    // Work out total syscall diff
    long nano = 1000000000;
    long totalTimeA = (tsA.tv_sec * nano) + tsA.tv_nsec;
    long totalTimeB = (tsB.tv_sec * nano) + tsB.tv_nsec;
    long diff = totalTimeB - totalTimeA;

    // Work out clock diff
    if(CLOCKS_PER_SEC != 1000000){
        printf("CLOCKS_PER_SEC not as expected (%li)", CLOCKS_PER_SEC);
    }
    double clockDiff = (double)(clockEnd - clockStart) / CLOCKS_PER_SEC;

    // Checks
    if (diff <= 0) {
        printf("Elapsed time from syscalls not greater later (diff = %lins)\n", diff);
        return 1;
    } else {
        printf("Elapsed time from syscalls greater later (diff = %lins)\n", diff);
    }

    if (clockDiff <= 0) {
        printf("Elapsed time from clock not greater later (diff = %fs)\n", clockDiff);
        return 1;
    } else {
        printf("Elapsed time from clock greater later (diff = %fs)\n", clockDiff);
    }

    return 0;
}
