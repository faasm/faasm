#include <eigen3/Eigen/Core>

#include "faasm/faasm.h"
#include "stdio.h"

/*
 * Invokes Eigen's config to check which SIMD instruction sets are in use
 */
FAASM_MAIN_FUNC() {
    const char* eigenInstrSets = Eigen::SimdInstructionSetsInUse();
    printf("Eigen SIMD instruction sets: %s\n", eigenInstrSets);

    return 0;
}