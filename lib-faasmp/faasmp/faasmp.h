#ifndef _FAASMP_H
#define _FAASMP_H

#include <stdint.h>


struct AlignedElem {
    int32_t i = 0;

    AlignedElem() = default;

    AlignedElem(int32_t i) : i(i) {};

    friend inline bool operator==(const AlignedElem &lhs, const AlignedElem &rhs) {
        return lhs.i == rhs.i;
    };
} __attribute__ ((aligned(128)));

#endif
