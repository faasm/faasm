#include <catch2/catch.hpp>

#include "faabric/scheduler/Scheduler.h"
#include "utils.h"

#include <faabric/util/func.h>

namespace tests {
void checkMpiFunc(const char* funcName)
{
    // Note: we don't `set_mpiworldsize` here, so all tests run with the default
    // MPI world size (5). Some tests will fail if we change this.
    faabric::Message msg = faabric::util::messageFactory("mpi", funcName);
    execFuncWithPool(msg, true);

    // Check all other functions were successful
    auto& sch = faabric::scheduler::getScheduler();
    for (auto m : sch.getRecordedMessagesAll()) {
        uint32_t messageId = msg.id();
        if (messageId == msg.id()) {
            // Already checked the main message ID
            continue;
        }

        const faabric::Message& result = sch.getFunctionResult(messageId, 1);

        if (result.returnvalue() != 0) {
            FAIL(fmt::format("Message ID {} failed", messageId));
        }
    }
}

TEST_CASE("Test MPI allgather", "[mpi]")
{
    checkMpiFunc("mpi_allgather");
}

TEST_CASE("Test MPI allreduce", "[mpi]")
{
    checkMpiFunc("mpi_allreduce");
}

TEST_CASE("Test MPI alltoall", "[mpi]")
{
    checkMpiFunc("mpi_alltoall");
}

TEST_CASE("Test MPI barrier", "[mpi]")
{
    checkMpiFunc("mpi_barrier");
}

TEST_CASE("Test MPI broadcast", "[mpi]")
{
    checkMpiFunc("mpi_bcast");
}

TEST_CASE("Test MPI cartesian create", "[mpi]")
{
    checkMpiFunc("mpi_cart_create");
}

TEST_CASE("Test MPI cartesian coordinates", "[mpi]")
{
    checkMpiFunc("mpi_cartesian");
}

TEST_CASE("Test general MPI functionality", "[mpi]")
{
    checkMpiFunc("mpi_checks");
}

TEST_CASE("Test MPI gather", "[mpi]")
{
    checkMpiFunc("mpi_gather");
}

TEST_CASE("Test MPI message ordering", "[mpi]")
{
    checkMpiFunc("mpi_order");
}

TEST_CASE("Test MPI one-sided comms", "[mpi]")
{
    checkMpiFunc("mpi_onesided");
}

TEST_CASE("Test MPI probe", "[mpi]")
{
    checkMpiFunc("mpi_probe");
}

TEST_CASE("Test MPI put", "[mpi]")
{
    checkMpiFunc("mpi_put");
}

TEST_CASE("Test MPI reduce", "[mpi]")
{
    checkMpiFunc("mpi_reduce");
}

TEST_CASE("Test MPI scan", "[mpi]")
{
    checkMpiFunc("mpi_scan");
}

TEST_CASE("Test MPI scatter", "[mpi]")
{
    checkMpiFunc("mpi_scatter");
}

TEST_CASE("Test MPI sendrecv", "[mpi]")
{
    checkMpiFunc("mpi_sendrecv");
}

TEST_CASE("Test MPI status", "[mpi]")
{
    checkMpiFunc("mpi_status");
}

TEST_CASE("Test MPI type sizes", "[mpi]")
{
    checkMpiFunc("mpi_typesize");
}

TEST_CASE("Test MPI window creation", "[mpi]")
{
    checkMpiFunc("mpi_wincreate");
}

// TEST_CASE("Test MPI async", "[mpi]")
//{
//    checkMpiFunc("mpi_isendrecv");
//}
}
