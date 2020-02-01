#ifndef FAASM_OMP_HELPER_H
#define FAASM_OMP_HELPER_H

struct AlignedElem {
    int32_t i;
    AlignedElem() : i{0} {};
    AlignedElem(int32_t i) : i{i} {};
    friend inline bool operator==(const AlignedElem& lhs, const AlignedElem& rhs) {
        return lhs.i == rhs.i;
    };
}__attribute__ ((aligned(128)));

#endif //FAASM_OMP_HELPER_H
