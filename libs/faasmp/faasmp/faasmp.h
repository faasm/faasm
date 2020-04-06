#ifndef _FAASMP_H
#define _FAASMP_H

#define CONCAT_(x,y) x##y
#define CONCAT(x,y) CONCAT_(x,y)
#define uniquename static bool CONCAT(sb_, __COUNTER__) = false

#include <cstdint>

#ifdef __cplusplus
extern "C" {
#endif

void __faasmp_debug_copy(int *a, int *b);

#ifdef __cplusplus
}
#endif

struct AlignedElem {
    int32_t i = 0;

    AlignedElem() = default;

    AlignedElem(int32_t i) : i(i) {};

    friend inline bool operator==(const AlignedElem &lhs, const AlignedElem &rhs) {
        return lhs.i == rhs.i;
    };
} __attribute__ ((aligned(128)));

class MultiHostSum {
    int64_t sum = 0;
public:
    MultiHostSum() = default;
    explicit  MultiHostSum(int initial) : sum(initial) {};
    MultiHostSum &operator+=(const MultiHostSum &other) {
        sum += other.sum;
        return *this;
    }
    // Should be private to Faasm only for security reason
    int num_devices = 0;

    int64_t get() {
        return sum;
    }
};

#pragma omp declare reduction(MultiHostSum: MultiHostSum: omp_out += omp_in)

#endif
