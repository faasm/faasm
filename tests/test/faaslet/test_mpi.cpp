#include <catch2/catch.hpp>

#include "faasm_fixtures.h"
#include "fixtures.h"
#include "utils.h"

#include <faabric/scheduler/Scheduler.h>
#include <faabric/util/environment.h>
#include <faabric/util/func.h>
#include <faabric/util/logging.h>
#include <faabric/util/string_tools.h>

namespace tests {

class MPIFuncTestFixture : public MultiRuntimeFunctionExecTestFixture
{
  public:
    MPIFuncTestFixture()
      : faasmConf(conf::getFaasmConfig())
    {
        oldWasmVm = faasmConf.wasmVm;
    }

    ~MPIFuncTestFixture() { faasmConf.wasmVm = oldWasmVm; }

    faabric::Message checkMpiFunc(const char* funcName)
    {
        // Prepare request
        int worldSize = 5;
        auto req = faabric::util::batchExecFactory("mpi", funcName, 1);
        req->mutable_messages(0)->set_ismpi(true);
        req->mutable_messages(0)->set_mpiworldsize(worldSize);

        // Execute it
        executeWithPool(req, 10000).at(0);

        // Return the first message's result for post-processing
        return plannerCli.getMessageResult(req->messages(0), 500);
    }

    conf::FaasmConfig& faasmConf;
    std::string oldWasmVm;
};

TEST_CASE_METHOD(MPIFuncTestFixture, "Test MPI allgather", "[mpi]")
{
    SECTION("WAVM") { faasmConf.wasmVm = "wavm"; }

    SECTION("WAMR") { faasmConf.wasmVm = "wamr"; }

    checkMpiFunc("mpi_allgather");
}

TEST_CASE_METHOD(MPIFuncTestFixture, "Test MPI allreduce", "[mpi]")
{
    SECTION("WAVM") { faasmConf.wasmVm = "wavm"; }

    SECTION("WAMR") { faasmConf.wasmVm = "wamr"; }

    checkMpiFunc("mpi_allreduce");
}

TEST_CASE_METHOD(MPIFuncTestFixture, "Test MPI alltoall", "[mpi]")
{
    SECTION("WAVM") { faasmConf.wasmVm = "wavm"; }

    SECTION("WAMR") { faasmConf.wasmVm = "wamr"; }

    checkMpiFunc("mpi_alltoall");
}

TEST_CASE_METHOD(MPIFuncTestFixture, "Test MPI barrier", "[mpi]")
{
    SECTION("WAVM") { faasmConf.wasmVm = "wavm"; }

    SECTION("WAMR") { faasmConf.wasmVm = "wamr"; }

    checkMpiFunc("mpi_barrier");
}

TEST_CASE_METHOD(MPIFuncTestFixture, "Test MPI broadcast", "[mpi]")
{
    SECTION("WAVM") { faasmConf.wasmVm = "wavm"; }

    SECTION("WAMR") { faasmConf.wasmVm = "wamr"; }

    checkMpiFunc("mpi_bcast");
}

TEST_CASE_METHOD(MPIFuncTestFixture, "Test MPI cartesian create", "[mpi]")
{
    SECTION("WAVM") { faasmConf.wasmVm = "wavm"; }

    SECTION("WAMR") { faasmConf.wasmVm = "wamr"; }

    checkMpiFunc("mpi_cart_create");
}

TEST_CASE_METHOD(MPIFuncTestFixture, "Test MPI cartesian coordinates", "[mpi]")
{
    SECTION("WAVM") { faasmConf.wasmVm = "wavm"; }

    SECTION("WAMR") { faasmConf.wasmVm = "wamr"; }

    checkMpiFunc("mpi_cartesian");
}

TEST_CASE_METHOD(MPIFuncTestFixture, "Test general MPI functionality", "[mpi]")
{
    SECTION("WAVM") { faasmConf.wasmVm = "wavm"; }

    SECTION("WAMR") { faasmConf.wasmVm = "wamr"; }

    checkMpiFunc("mpi_checks");
}

TEST_CASE_METHOD(MPIFuncTestFixture, "Test MPI gather", "[mpi]")
{
    SECTION("WAVM") { faasmConf.wasmVm = "wavm"; }

    SECTION("WAMR") { faasmConf.wasmVm = "wamr"; }

    checkMpiFunc("mpi_gather");
}

TEST_CASE_METHOD(MPIFuncTestFixture, "Test MPI message ordering", "[mpi]")
{
    SECTION("WAVM") { faasmConf.wasmVm = "wavm"; }

    SECTION("WAMR") { faasmConf.wasmVm = "wamr"; }

    checkMpiFunc("mpi_order");
}

TEST_CASE_METHOD(MPIFuncTestFixture, "Test MPI reduce", "[mpi]")
{
    SECTION("WAVM") { faasmConf.wasmVm = "wavm"; }

    SECTION("WAMR") { faasmConf.wasmVm = "wamr"; }

    checkMpiFunc("mpi_reduce");
}

TEST_CASE_METHOD(MPIFuncTestFixture, "Test MPI scan", "[mpi]")
{
    SECTION("WAVM") { faasmConf.wasmVm = "wavm"; }

    SECTION("WAMR") { faasmConf.wasmVm = "wamr"; }

    checkMpiFunc("mpi_scan");
}

TEST_CASE_METHOD(MPIFuncTestFixture, "Test MPI scatter", "[mpi]")
{
    SECTION("WAVM") { faasmConf.wasmVm = "wavm"; }

    SECTION("WAMR") { faasmConf.wasmVm = "wamr"; }

    checkMpiFunc("mpi_scatter");
}

TEST_CASE_METHOD(MPIFuncTestFixture, "Test MPI sendrecv", "[mpi]")
{
    SECTION("WAVM") { faasmConf.wasmVm = "wavm"; }

    SECTION("WAMR") { faasmConf.wasmVm = "wamr"; }

    checkMpiFunc("mpi_sendrecv");
}

TEST_CASE_METHOD(MPIFuncTestFixture, "Test MPI status", "[mpi]")
{
    SECTION("WAVM") { faasmConf.wasmVm = "wavm"; }

    SECTION("WAMR") { faasmConf.wasmVm = "wamr"; }

    checkMpiFunc("mpi_status");
}

TEST_CASE_METHOD(MPIFuncTestFixture, "Test MPI type sizes", "[mpi]")
{
    SECTION("WAVM") { faasmConf.wasmVm = "wavm"; }

    SECTION("WAMR") { faasmConf.wasmVm = "wamr"; }

    checkMpiFunc("mpi_typesize");
}

TEST_CASE_METHOD(MPIFuncTestFixture, "Test MPI async", "[mpi]")
{
    SECTION("WAVM") { faasmConf.wasmVm = "wavm"; }

    SECTION("WAMR") { faasmConf.wasmVm = "wamr"; }

    checkMpiFunc("mpi_isendrecv");
}

TEST_CASE_METHOD(MPIFuncTestFixture, "Test MPI Pi", "[mpi]")
{
    SECTION("WAVM") { faasmConf.wasmVm = "wavm"; }

    SECTION("WAMR") { faasmConf.wasmVm = "wamr"; }

    faabric::Message result = checkMpiFunc("mpi_pi");
    std::string output = result.outputdata();
    REQUIRE(faabric::util::startsWith(output, "Pi estimate: 3.1"));
}
}
