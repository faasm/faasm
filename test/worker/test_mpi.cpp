#include <catch/catch.hpp>

#include "utils.h"

#include <util/func.h>

namespace tests {
    TEST_CASE("Test running MPI checks", "[wasm]") {
        cleanSystem();
        message::Message msg = util::messageFactory("mpi", "mpi_checks");
        execFuncWithPool(msg, false, 1, true);
    }

    TEST_CASE("Test running MPI status check", "[wasm]") {
        cleanSystem();
        message::Message msg = util::messageFactory("mpi", "mpi_status");
        execFuncWithPool(msg, false, 1, true);
    }

    TEST_CASE("Test running MPI probe check", "[wasm]") {
        cleanSystem();
        message::Message msg = util::messageFactory("mpi", "mpi_probe");
        execFuncWithPool(msg, false, 1, true);
    }
}