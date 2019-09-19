#ifndef FAASM_TIME_H
#define FAASM_TIME_H

#include <stdio.h>
#include <time.h>

// Turn on timings when not building wasm or when WASM_PROF requested
#if WASM_PROF == 1 || __WASM__ != 1
#define FAASM_PROF_START(profVar) clock_t profVar = clock();
#define FAASM_PROF_END(profVar) float diff_##profVar = ((clock() - profVar) * 1000.0f) / CLOCKS_PER_SEC; \
                                printf("TIME: %s %f\n", #profVar, diff_##profVar);
#else
#define FAASM_PROF_START(profVar)
#define FAASM_PROF_END(profVar)
#endif

namespace faasm {
    double getSecondsSinceEpoch();
}

#endif
