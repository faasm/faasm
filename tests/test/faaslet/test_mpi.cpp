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

class MPIFuncTestFixture
  : public FunctionExecTestFixture
  , public MpiBaseTestFixture
{
  public:
    faabric::Message checkMpiFunc(const char* funcName)
    {
        // Note: we don't `set_mpiworldsize` here, so all tests run with the
        // default MPI world size (5). Some tests will fail if we change this.
        faabric::Message msg = faabric::util::messageFactory("mpi", funcName);
        faabric::Message result = execFuncWithPool(msg, true, 10000);

        // Check all other functions were successful
        auto& sch = faabric::scheduler::getScheduler();
        for (auto m : sch.getRecordedMessagesAll()) {
            uint32_t messageId = msg.id();
            if (messageId == msg.id()) {
                // Already checked the main message ID
                continue;
            }

            faabric::Message result = sch.getFunctionResult(messageId, 1);

            if (result.returnvalue() != 0) {
                FAIL(fmt::format("Message ID {} failed", messageId));
            }
        }

        return result;
    }
};

TEST_CASE_METHOD(MPIFuncTestFixture, "Test MPI allgather", "[mpi]")
{
    checkMpiFunc("mpi_allgather");
}

TEST_CASE_METHOD(MPIFuncTestFixture, "Test MPI allreduce", "[mpi]")
{
    checkMpiFunc("mpi_allreduce");
}

TEST_CASE_METHOD(MPIFuncTestFixture, "Test MPI alltoall", "[mpi]")
{
    checkMpiFunc("mpi_alltoall");
}

TEST_CASE_METHOD(MPIFuncTestFixture, "Test MPI barrier", "[mpi]")
{
    checkMpiFunc("mpi_barrier");
}

TEST_CASE_METHOD(MPIFuncTestFixture, "Test MPI broadcast", "[mpi]")
{
    checkMpiFunc("mpi_bcast");
}

TEST_CASE_METHOD(MPIFuncTestFixture, "Test MPI cartesian create", "[mpi]")
{
    checkMpiFunc("mpi_cart_create");
}

TEST_CASE_METHOD(MPIFuncTestFixture, "Test MPI cartesian coordinates", "[mpi]")
{
    checkMpiFunc("mpi_cartesian");
}

TEST_CASE_METHOD(MPIFuncTestFixture, "Test general MPI functionality", "[mpi]")
{
    checkMpiFunc("mpi_checks");
}

TEST_CASE_METHOD(MPIFuncTestFixture, "Test MPI gather", "[mpi]")
{
    checkMpiFunc("mpi_gather");
}

TEST_CASE_METHOD(MPIFuncTestFixture, "Test MPI message ordering", "[mpi]")
{
    checkMpiFunc("mpi_order");
}

// 31/12/21 - Probe support is broken after faasm/faabric#205
TEST_CASE_METHOD(MPIFuncTestFixture, "Test MPI probe", "[.]")
{
    checkMpiFunc("mpi_probe");
}

TEST_CASE_METHOD(MPIFuncTestFixture, "Test MPI reduce", "[mpi]")
{
    checkMpiFunc("mpi_reduce");
}

TEST_CASE_METHOD(MPIFuncTestFixture, "Test MPI scan", "[mpi]")
{
    checkMpiFunc("mpi_scan");
}

TEST_CASE_METHOD(MPIFuncTestFixture, "Test MPI scatter", "[mpi]")
{
    checkMpiFunc("mpi_scatter");
}

TEST_CASE_METHOD(MPIFuncTestFixture, "Test MPI sendrecv", "[mpi]")
{
    checkMpiFunc("mpi_sendrecv");
}

TEST_CASE_METHOD(MPIFuncTestFixture, "Test MPI status", "[mpi]")
{
    checkMpiFunc("mpi_status");
}

TEST_CASE_METHOD(MPIFuncTestFixture, "Test MPI type sizes", "[mpi]")
{
    checkMpiFunc("mpi_typesize");
}

TEST_CASE_METHOD(MPIFuncTestFixture, "Test MPI async", "[mpi]")
{
    checkMpiFunc("mpi_isendrecv");
}

TEST_CASE_METHOD(MPIFuncTestFixture, "Test MPI Pi", "[mpi]")
{
    faabric::Message result = checkMpiFunc("mpi_pi");
    std::string output = result.outputdata();
    REQUIRE(faabric::util::startsWith(output, "Pi estimate: 3.1"));
}
}
