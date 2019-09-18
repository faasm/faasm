#include <eigen3/Eigen/Core>

#include "faasm/faasm.h"
#include "stdio.h"

FAASM_MAIN_FUNC() {
    Eigen::internal::QuadPacket a;
    Eigen::internal::QuadPacket b;
    Eigen::internal::padd(a, b);
    return 0;
}