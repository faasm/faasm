#include <catch2/catch.hpp>

#include "faasm_fixtures.h"
#include "fixtures.h"
#include "utils.h"

namespace tests {
// We read the different command line arguments from PRK's CI scripts
TEST_CASE_METHOD(FunctionExecTestFixture,
                 "Test executing MPI kernels",
                 "[libs]")
{
    std::string kernelName;
    std::string cmdLine;

    SECTION("Kernel: global")
    {
        kernelName = "global";
        cmdLine = "10 1024";
    }

    SECTION("Kernel: p2p")
    {
        kernelName = "p2p";
        cmdLine = "10 1024 1024";
    }

    SECTION("Kernel: sparse")
    {
        kernelName = "sparse";
        cmdLine = "10 10 5";
    }

    SECTION("Kernel: transpose")
    {
        kernelName = "transpose";
        cmdLine = "10 1024 32";
    }

    SECTION("Kernel: stencil")
    {
        kernelName = "stencil";
        cmdLine = "10 1000";
    }

    /* The dgemm MPI kernel does not work because we are missing
     * implementations for MPI_Comm_group, MPI_Group_incl, and MPI_Comm_create
    SECTION("Kernel: dgemm")
    {
        kernelName = "dgemm";
        cmdLine = "10 1024 32 1";
    }
    */

    SECTION("Kernel: nstream")
    {
        kernelName = "nstream";
        cmdLine = "10 1024 32";
    }

    SECTION("Kernel: reduce")
    {
        kernelName = "reduce";
        cmdLine = "10 1024";
    }

    /* The random MPI kernel does not work because we are missing
     * implementations for MPI_Alltoallv
    SECTION("Kernel: random")
    {
        kernelName = "random";
        cmdLine = "32 20";
    }
    */

    auto req = setUpContext("kernels-mpi", kernelName);
    faabric::Message& msg = req->mutable_messages()->at(0);
    msg.set_cmdline(cmdLine);
    msg.set_mpiworldsize(2);
    execFunction(msg);
}

TEST_CASE_METHOD(OpenMPTestFixture, "Test executing OpenMP kernels", "[libs]")
{
    std::string kernelName;
    std::string cmdLine;

    SECTION("Kernel: global")
    {
        kernelName = "global";
        cmdLine = "2 10 1024";
    }

    SECTION("Kernel: p2p")
    {
        kernelName = "p2p";
        cmdLine = "2 10 1024 1024";
    }

    /* The sparse OpenMP kernel fails during the result integrity checks
    SECTION("Kernel: sparse")
    {
        kernelName = "sparse";
        cmdLine = "2 10 10 5";
    }
    */

    /* The transpose OpenMP kernel can not be cross-compiled due to problems
     * with the static_for signature and symbol definition
    SECTION("Kernel: transpose")
    {
        kernelName = "transpose";
        cmdLine = "10 1024 32";
    }
    */

    /* The stencil OpenMP kernel can not be cross-compiled due to a problem
     * with the faasm_sm_reduce signature
    SECTION("Kernel: stencil")
    {
        kernelName = "stencil";
        cmdLine = "10 1000";
    }
    */

    /* The dgemm MPI kernel does not work because we are missing
     * implementations for MPI_Comm_group, MPI_Group_incl, and MPI_Comm_create
    SECTION("Kernel: dgemm")
    {
        kernelName = "dgemm";
        cmdLine = "10 1024 32 1";
    }
    */

    SECTION("Kernel: nstream")
    {
        kernelName = "nstream";
        cmdLine = "2 10 1024 32";
    }

    SECTION("Kernel: reduce")
    {
        kernelName = "reduce";
        cmdLine = "2 10 1024";
    }

    /* The random OpenMP kernel can not be cross-compiled due to a problem
     * with the faasm_sm_reduce signature
    SECTION("Kernel: random")
    {
        kernelName = "random";
        cmdLine = "32 20";
    }
    */

    auto req = setUpContext("kernels-omp", kernelName);
    faabric::Message& msg = req->mutable_messages()->at(0);
    msg.set_cmdline(cmdLine);
    execFuncWithPool(msg, false, OMP_TEST_TIMEOUT_MS);
}
}
