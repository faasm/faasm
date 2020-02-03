#include <catch/catch.hpp>

#include "utils.h"

#include <util/func.h>

namespace tests {
    TEST_CASE("Test running MPI checks", "[wasm]") {
        cleanSystem();
        message::Message msg = util::messageFactory("mpi", "mpi_checks");
//        execFuncWithPool(msg);
    }
}