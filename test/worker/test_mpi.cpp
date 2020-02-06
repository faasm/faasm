#include <catch/catch.hpp>

#include "utils.h"

#include <util/func.h>

namespace tests {
    void checkMpiFunc(const char* funcName) {
        message::Message msg = util::messageFactory("mpi", funcName);
        execFuncWithPool(msg, false, 1, true, 10);
    }

    TEST_CASE("Test MPI broadcast", "[wasm]") {
        checkMpiFunc("mpi_bcast");
    }

    TEST_CASE("Test running MPI checks", "[wasm]") {
        checkMpiFunc("mpi_checks");
    }

    TEST_CASE("Test running MPI order check", "[wasm]") {
        checkMpiFunc("mpi_order");
    }

    TEST_CASE("Test running MPI probe check", "[wasm]") {
        checkMpiFunc("mpi_probe");
    }

    TEST_CASE("Test running MPI status check", "[wasm]") {
        checkMpiFunc("mpi_status");
    }
}