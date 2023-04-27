#include <catch2/catch.hpp>

#include "faasm_fixtures.h"
#include "fixtures.h"
#include "utils.h"

#include <faabric/runner/FaabricMain.h>
#include <faabric/scheduler/Scheduler.h>
#include <faabric/util/environment.h>
#include <faabric/util/func.h>
#include <faabric/util/logging.h>
#include <faabric/util/string_tools.h>

namespace tests {

class MPIFuncTestFixture : public MultiRuntimeFunctionExecTestFixture
  // , public MpiBaseTestFixture
{
  public:
    MPIFuncTestFixture()
      : fac(std::make_shared<faaslet::FaasletFactory>())
      , m(fac)
      , worldSize(5)
    {
        auto& mpiRegistry = faabric::mpi::getMpiWorldRegistry();
        mpiRegistry.clear();

        // TODO: this probably should happen in the FunctionExecTestFixture
        m.startBackground();
    }

    ~MPIFuncTestFixture()
    {
        auto& mpiRegistry = faabric::mpi::getMpiWorldRegistry();
        mpiRegistry.clear();

        m.shutdown();
    }

    void checkMpiFunc(const char* funcName, std::string* captureOut = nullptr)
    {
        auto& sch = faabric::scheduler::getScheduler();

        // Note: we don't `set_mpiworldsize` here, so all tests run with the
        // default MPI world size (5). Some tests will fail if we change this.
        auto req = faabric::util::batchExecFactory("mpi", funcName, 1);
        sch.callFunctions(req);

        int nRetries = 3;
        auto reqResp = sch.getBatchResult(req, 10000, worldSize);
        while (!reqResp && nRetries > 0) {
            --nRetries;
            SLEEP_MS(2000);
            reqResp = sch.getBatchResult(req, 10000, worldSize);
        }

        REQUIRE(reqResp != nullptr);
        for (const auto& msg : reqResp->messages()) {
            REQUIRE(msg.returnvalue() == 0);
        }

        if (captureOut) {
            *captureOut = reqResp->messages(0).outputdata();
        }
    }

  protected:
    std::shared_ptr<faaslet::FaasletFactory> fac;
    faabric::runner::FaabricMain m;
    int worldSize;
};

TEST_CASE_METHOD(MPIFuncTestFixture, "Test MPI allgather", "[mpi]")
{
    SECTION("WAVM") { conf.wasmVm = "wavm"; }

    SECTION("WAMR") { conf.wasmVm = "wamr"; }

    checkMpiFunc("mpi_allgather");
}

TEST_CASE_METHOD(MPIFuncTestFixture, "Test MPI allreduce", "[mpi]")
{
    SECTION("WAVM") { conf.wasmVm = "wavm"; }

    SECTION("WAMR") { conf.wasmVm = "wamr"; }

    checkMpiFunc("mpi_allreduce");
}

TEST_CASE_METHOD(MPIFuncTestFixture, "Test MPI alltoall", "[mpi]")
{
    SECTION("WAVM") { conf.wasmVm = "wavm"; }

    SECTION("WAMR") { conf.wasmVm = "wamr"; }

    checkMpiFunc("mpi_alltoall");
}

TEST_CASE_METHOD(MPIFuncTestFixture, "Test MPI barrier", "[mpi]")
{
    SECTION("WAVM") { conf.wasmVm = "wavm"; }

    SECTION("WAMR") { conf.wasmVm = "wamr"; }

    checkMpiFunc("mpi_barrier");
}

TEST_CASE_METHOD(MPIFuncTestFixture, "Test MPI broadcast", "[mpi]")
{
    SECTION("WAVM") { conf.wasmVm = "wavm"; }

    SECTION("WAMR") { conf.wasmVm = "wamr"; }

    checkMpiFunc("mpi_bcast");
}

TEST_CASE_METHOD(MPIFuncTestFixture, "Test MPI cartesian create", "[mpi]")
{
    SECTION("WAVM") { conf.wasmVm = "wavm"; }

    SECTION("WAMR") { conf.wasmVm = "wamr"; }

    checkMpiFunc("mpi_cart_create");
}

TEST_CASE_METHOD(MPIFuncTestFixture, "Test MPI cartesian coordinates", "[mpi]")
{
    SECTION("WAVM") { conf.wasmVm = "wavm"; }

    SECTION("WAMR") { conf.wasmVm = "wamr"; }

    checkMpiFunc("mpi_cartesian");
}

TEST_CASE_METHOD(MPIFuncTestFixture, "Test general MPI functionality", "[mpi]")
{
    SECTION("WAVM") { conf.wasmVm = "wavm"; }

    SECTION("WAMR") { conf.wasmVm = "wamr"; }

    checkMpiFunc("mpi_checks");
}

TEST_CASE_METHOD(MPIFuncTestFixture, "Test MPI gather", "[mpi]")
{
    SECTION("WAVM") { conf.wasmVm = "wavm"; }

    SECTION("WAMR") { conf.wasmVm = "wamr"; }

    checkMpiFunc("mpi_gather");
}

TEST_CASE_METHOD(MPIFuncTestFixture, "Test MPI message ordering", "[mpi]")
{
    SECTION("WAVM") { conf.wasmVm = "wavm"; }

    SECTION("WAMR") { conf.wasmVm = "wamr"; }

    checkMpiFunc("mpi_order");
}

// 31/12/21 - Probe support is broken after faasm/faabric#205
/*
TEST_CASE_METHOD(MPIFuncTestFixture, "Test MPI probe", "[.]")
{
    SECTION("WAVM") { conf.wasmVm = "wavm"; }

    SECTION("WAMR") { conf.wasmVm = "wamr"; }

    checkMpiFunc("mpi_probe");
}
*/

TEST_CASE_METHOD(MPIFuncTestFixture, "Test MPI reduce", "[mpi]")
{
    SECTION("WAVM") { conf.wasmVm = "wavm"; }

    SECTION("WAMR") { conf.wasmVm = "wamr"; }

    checkMpiFunc("mpi_reduce");
}

TEST_CASE_METHOD(MPIFuncTestFixture, "Test MPI scan", "[mpi]")
{
    SECTION("WAVM") { conf.wasmVm = "wavm"; }

    SECTION("WAMR") { conf.wasmVm = "wamr"; }

    checkMpiFunc("mpi_scan");
}

TEST_CASE_METHOD(MPIFuncTestFixture, "Test MPI scatter", "[mpi]")
{
    SECTION("WAVM") { conf.wasmVm = "wavm"; }

    SECTION("WAMR") { conf.wasmVm = "wamr"; }

    checkMpiFunc("mpi_scatter");
}

TEST_CASE_METHOD(MPIFuncTestFixture, "Test MPI sendrecv", "[mpi]")
{
    SECTION("WAVM") { conf.wasmVm = "wavm"; }

    SECTION("WAMR") { conf.wasmVm = "wamr"; }

    checkMpiFunc("mpi_sendrecv");
}

TEST_CASE_METHOD(MPIFuncTestFixture, "Test MPI status", "[mpi]")
{
    SECTION("WAVM") { conf.wasmVm = "wavm"; }

    SECTION("WAMR") { conf.wasmVm = "wamr"; }

    checkMpiFunc("mpi_status");
}

TEST_CASE_METHOD(MPIFuncTestFixture, "Test MPI type sizes", "[mpi]")
{
    SECTION("WAVM") { conf.wasmVm = "wavm"; }

    SECTION("WAMR") { conf.wasmVm = "wamr"; }

    checkMpiFunc("mpi_typesize");
}

TEST_CASE_METHOD(MPIFuncTestFixture, "Test MPI async", "[mpi]")
{
    SECTION("WAVM") { conf.wasmVm = "wavm"; }

    SECTION("WAMR") { conf.wasmVm = "wamr"; }

    checkMpiFunc("mpi_isendrecv");
}

TEST_CASE_METHOD(MPIFuncTestFixture, "Test MPI Pi", "[mpi]")
{
    SECTION("WAVM") { conf.wasmVm = "wavm"; }

    SECTION("WAMR") { conf.wasmVm = "wamr"; }

    std::string output;
    checkMpiFunc("mpi_pi", &output);
    REQUIRE(faabric::util::startsWith(output, "Pi estimate: 3.1"));
}
}
