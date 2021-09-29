#include <catch2/catch.hpp>

#include "faasm_fixtures.h"
#include "utils.h"

#include <faabric/proto/faabric.pb.h>
#include <faabric/scheduler/Scheduler.h>
#include <faabric/snapshot/SnapshotRegistry.h>
#include <faabric/util/config.h>
#include <faabric/util/environment.h>
#include <faabric/util/func.h>

// Longer timeout to allow longer-running functions to finish even when doing
// trace-level logging
#define OMP_TEST_TIMEOUT_MS 60000

namespace tests {

class OmpLocalTestFixture
  : public FunctionExecTestFixture
  , public ConfTestFixture
{
  public:
    void doOmpTestLocal(const std::string& function)
    {
        conf.overrideCpuCount = 15;

        faabric::Message msg = faabric::util::messageFactory("omp", function);
        execFuncWithPool(msg, false, OMP_TEST_TIMEOUT_MS);
    }
};

TEST_CASE_METHOD(OmpLocalTestFixture,
                 "Test static for scheduling",
                 "[wasm][openmp]")
{
    doOmpTestLocal("for_static_schedule");
}

TEST_CASE_METHOD(OmpLocalTestFixture,
                 "Test OMP header API functions",
                 "[wasm][openmp]")
{
    doOmpTestLocal("header_api_support");
}

TEST_CASE_METHOD(OmpLocalTestFixture,
                 "Test running OpenMP checks",
                 "[wasm][openmp]")
{
    doOmpTestLocal("omp_checks");
}

TEST_CASE_METHOD(OmpLocalTestFixture,
                 "Test non-nested barrier pragma",
                 "[wasm][openmp]")
{
    doOmpTestLocal("simple_barrier");
}

TEST_CASE_METHOD(OmpLocalTestFixture,
                 "Test basic omp parallel for pragma",
                 "[wasm][openmp]")
{
    doOmpTestLocal("simple_for");
}

TEST_CASE_METHOD(OmpLocalTestFixture,
                 "Test non-nested master pragma",
                 "[wasm][openmp]")
{
    doOmpTestLocal("simple_master");
}

TEST_CASE_METHOD(OmpLocalTestFixture,
                 "Test simple reduction function",
                 "[wasm][openmp]")
{
    doOmpTestLocal("simple_reduce");
}

TEST_CASE_METHOD(OmpLocalTestFixture,
                 "Test integrating using different constructs",
                 "[wasm][openmp]")
{
    doOmpTestLocal("reduction_integral");
}

TEST_CASE_METHOD(OmpLocalTestFixture, "Test critical section", "[wasm][openmp]")
{
    doOmpTestLocal("simple_critical");
}

TEST_CASE_METHOD(OmpLocalTestFixture, "Test single section", "[wasm][openmp]")
{
    doOmpTestLocal("simple_single");
}

TEST_CASE_METHOD(OmpLocalTestFixture,
                 "Test custom reduction function",
                 "[wasm][openmp]")
{
    doOmpTestLocal("custom_reduce");
}

TEST_CASE_METHOD(OmpLocalTestFixture, "Test nested API", "[wasm][openmp]")
{
    doOmpTestLocal("nested_levels_test");
}

TEST_CASE_METHOD(OmpLocalTestFixture,
                 "Test nested parallel region support",
                 "[wasm][openmp]")
{
    doOmpTestLocal("nested_parallel");
}

TEST_CASE_METHOD(OmpLocalTestFixture,
                 "Test openmp Pi calculation",
                 "[wasm][openmp]")
{
    doOmpTestLocal("mt_pi");
}

TEST_CASE_METHOD(OmpLocalTestFixture,
                 "Test getting and setting num threads",
                 "[wasm][openmp]")
{
    doOmpTestLocal("setting_num_threads");
}

TEST_CASE_METHOD(OmpLocalTestFixture, "Test openmp wtime", "[wasm][openmp]")
{
    doOmpTestLocal("wtime");
}

TEST_CASE_METHOD(OmpLocalTestFixture,
                 "Test single-threaded reduction",
                 "[wasm][openmp]")
{
    doOmpTestLocal("single_thread_reduce");
}

TEST_CASE_METHOD(OmpLocalTestFixture,
                 "Test more complex reduction",
                 "[wasm][openmp]")
{
    doOmpTestLocal("complex_reduce");
}

TEST_CASE_METHOD(OmpLocalTestFixture,
                 "Test repeated reductions",
                 "[wasm][openmp]")
{
    doOmpTestLocal("repeated_reduce");
}

TEST_CASE_METHOD(OmpLocalTestFixture,
                 "Run openmp memory stress test",
                 "[wasm][openmp]")
{
    // Overload the local resources
    int nSlots = 15;
    int nOmpThreads = 60;

    faabric::scheduler::Scheduler& sch = faabric::scheduler::getScheduler();
    faabric::HostResources res;
    res.set_slots(nSlots);
    sch.setThisHostResources(res);

    // Overload the number of cores
    faabric::Message msg = faabric::util::messageFactory("omp", "mem_stress");
    msg.set_cmdline(std::to_string(nOmpThreads));

    execFuncWithPool(msg, false, OMP_TEST_TIMEOUT_MS);
}
}
