#include <catch/catch.hpp>

#include "utils.h"

#include <util/func.h>

namespace tests {
    void checkMpiFunc(const char* funcName) {
        message::Message msg = util::messageFactory("mpi", funcName);
        execFuncWithPool(msg, false, 1, true, 10);
    }

    TEST_CASE("Test MPI allgather", "[wasm]") {
        checkMpiFunc("mpi_allgather");
    }

    TEST_CASE("Test MPI allreduce", "[wasm]") {
        checkMpiFunc("mpi_allreduce");
    }

    TEST_CASE("Test MPI alltoall", "[wasm]") {
        checkMpiFunc("mpi_alltoall");
    }

    TEST_CASE("Test MPI barrier", "[wasm]") {
        checkMpiFunc("mpi_barrier");
    }

    TEST_CASE("Test MPI broadcast", "[wasm]") {
        checkMpiFunc("mpi_bcast");
    }

    TEST_CASE("Test general MPI functionality", "[wasm]") {
        checkMpiFunc("mpi_checks");
    }

    TEST_CASE("Test MPI gather", "[wasm]") {
        checkMpiFunc("mpi_gather");
    }

    TEST_CASE("Test MPI message ordering", "[wasm]") {
        checkMpiFunc("mpi_order");
    }

    TEST_CASE("Test MPI one-sided comms", "[wasm]") {
        checkMpiFunc("mpi_onesided");
    }

    TEST_CASE("Test MPI probe", "[wasm]") {
        checkMpiFunc("mpi_probe");
    }

    TEST_CASE("Test MPI reduce", "[wasm]") {
        checkMpiFunc("mpi_reduce");
    }

    TEST_CASE("Test MPI scatter", "[wasm]") {
        checkMpiFunc("mpi_scatter");
    }

    TEST_CASE("Test MPI status", "[wasm]") {
        checkMpiFunc("mpi_status");
    }
}