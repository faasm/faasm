#include <catch2/catch.hpp>

#include "faasm_fixtures.h"
#include "fixtures.h"
#include "utils.h"

#include <faabric/proto/faabric.pb.h>
#include <faabric/scheduler/Scheduler.h>
#include <faabric/snapshot/SnapshotRegistry.h>
#include <faabric/util/config.h>
#include <faabric/util/environment.h>
#include <faabric/util/func.h>
#include <faabric/util/string_tools.h>

// Longer timeout to allow longer-running functions to finish even when doing
// trace-level logging
#define OMP_TEST_TIMEOUT_MS 60000

namespace tests {

class OpenMPTestFixture
  : public FunctionExecTestFixture
  , public SnapshotRegistryFixture
  , public FaasmConfTestFixture
  , public ConfFixture
{
  public:
    OpenMPTestFixture()
    {
        faabric::HostResources res;
        res.set_slots(5);
        sch.setThisHostResources(res);
    }

    ~OpenMPTestFixture() {}

    std::string doOmpTestLocal(const std::string& function)
    {
        faabric::Message msg = faabric::util::messageFactory("omp", function);
        auto req = faabric::util::batchExecFactory("omp", function, 1);
        req->set_singlehosthint(true);

        // 08/03/2024 - Some local OpenMP tests stopped working with WAVM
        // after upgrade to LLVM 17 due to WAVM's incorrect handling of
        // atomics
        // SECTION("WAVM") { faasmConf.wasmVm = "wavm"; }

        SECTION("WAMR")
        {
            faasmConf.wasmVm = "wamr";
        }

        faabric::Message result =
          executeWithPool(req, OMP_TEST_TIMEOUT_MS).at(0);

        return result.outputdata();
    }
};

TEST_CASE_METHOD(OpenMPTestFixture,
                 "Test OpenMP static for scheduling",
                 "[wasm][openmp]")
{
    doOmpTestLocal("for_static_schedule");
}

TEST_CASE_METHOD(OpenMPTestFixture,
                 "Test OpenMP header API functions",
                 "[wasm][openmp]")
{
    doOmpTestLocal("header_api_support");
}

TEST_CASE_METHOD(OpenMPTestFixture,
                 "Test OpenMP general conformance",
                 "[wasm][openmp]")
{
    doOmpTestLocal("omp_checks");
}

TEST_CASE_METHOD(OpenMPTestFixture,
                 "Test OpenMP barrier pragma",
                 "[wasm][openmp]")
{
    doOmpTestLocal("simple_barrier");
}

TEST_CASE_METHOD(OpenMPTestFixture,
                 "Test OpenMP parallel for pragma",
                 "[wasm][openmp]")
{
    doOmpTestLocal("simple_for");
}

TEST_CASE_METHOD(OpenMPTestFixture,
                 "Test OpenMP master pragma",
                 "[wasm][openmp]")
{
    doOmpTestLocal("simple_master");
}

TEST_CASE_METHOD(OpenMPTestFixture, "Test OpenMP reduction", "[wasm][openmp]")
{
    doOmpTestLocal("simple_reduce");
}

TEST_CASE_METHOD(OpenMPTestFixture,
                 "Test a mix of OpenMP constructs",
                 "[wasm][openmp]")
{
    faabric::HostResources res;
    res.set_slots(10);
    sch.setThisHostResources(res);

    // Function requires 10 slots
    doOmpTestLocal("reduction_integral");
}

TEST_CASE_METHOD(OpenMPTestFixture,
                 "Test OpenMP critical section",
                 "[wasm][openmp]")
{
    doOmpTestLocal("simple_critical");
}

TEST_CASE_METHOD(OpenMPTestFixture,
                 "Test OpenMP single section",
                 "[wasm][openmp]")
{
    faabric::HostResources res;
    res.set_slots(100);
    sch.setThisHostResources(res);

    doOmpTestLocal("simple_single");
}

TEST_CASE_METHOD(OpenMPTestFixture,
                 "Test custom OpenMP reduction function",
                 "[wasm][openmp]")
{
    faabric::HostResources res;
    res.set_slots(10);
    sch.setThisHostResources(res);

    doOmpTestLocal("custom_reduce");
}

TEST_CASE_METHOD(OpenMPTestFixture,
                 "Test OpenMP Pi calculation using libfaasm",
                 "[wasm][openmp]")
{
    std::string output = doOmpTestLocal("pi_faasm");

    // Just check Pi to one dp
    REQUIRE(faabric::util::startsWith(output, "Pi estimate: 3.1"));
}

TEST_CASE_METHOD(OpenMPTestFixture,
                 "Test getting and setting number of OpenMP threads",
                 "[wasm][openmp]")
{
    faabric::HostResources res;
    res.set_slots(200);
    sch.setThisHostResources(res);

    doOmpTestLocal("setting_num_threads");
}

TEST_CASE_METHOD(OpenMPTestFixture, "Test OpenMP wtime", "[wasm][openmp]")
{
    doOmpTestLocal("wtime");
}

TEST_CASE_METHOD(OpenMPTestFixture,
                 "Test single-threaded OpenMP reduction",
                 "[wasm][openmp]")
{
    doOmpTestLocal("single_thread_reduce");
}

TEST_CASE_METHOD(OpenMPTestFixture,
                 "Test more complex OpenMP reduction",
                 "[wasm][openmp]")
{
    doOmpTestLocal("complex_reduce");
}

TEST_CASE_METHOD(OpenMPTestFixture,
                 "Test repeated OpenMP reductions",
                 "[wasm][openmp]")
{
    faabric::HostResources res;
    res.set_slots(10);
    sch.setThisHostResources(res);

    doOmpTestLocal("repeated_reduce");
}

TEST_CASE_METHOD(OpenMPTestFixture, "Test OpenMP atomic", "[wasm][openmp]")
{
    faabric::HostResources res;
    res.set_slots(200);
    sch.setThisHostResources(res);

    doOmpTestLocal("simple_atomic");
}

TEST_CASE_METHOD(OpenMPTestFixture,
                 "Test OpenMP default shared",
                 "[wasm][openmp]")
{
    doOmpTestLocal("default_shared");
}

TEST_CASE_METHOD(OpenMPTestFixture,
                 "Run OpenMP memory stress test",
                 "[wasm][openmp][.]")
{
    // Overload the local resources
    int nOmpThreads = 10;

    faabric::HostResources res;
    res.set_slots(20);
    sch.setThisHostResources(res);

    // Overload the number of cores
    faabric::Message msg = faabric::util::messageFactory("omp", "mem_stress");
    auto req = faabric::util::batchExecFactory("omp", "mem_stress", 1);
    req->mutable_messages(0)->set_cmdline(std::to_string(nOmpThreads));
    req->set_singlehosthint(true);

    SECTION("WAMR")
    {
        faasmConf.wasmVm = "wamr";
    }

    auto result = executeWithPool(req, OMP_TEST_TIMEOUT_MS).at(0);
    REQUIRE(result.returnvalue() == 0);
}

TEST_CASE_METHOD(OpenMPTestFixture,
                 "Test nested OpenMP explicitly disabled",
                 "[wasm][openmp]")
{
    // Make sure there's definitely enough slots
    int nSlots = 20;
    faabric::HostResources res;
    res.set_slots(nSlots);
    sch.setThisHostResources(res);

    auto req = faabric::util::batchExecFactory("omp", "nested_parallel", 1);
    req->set_singlehosthint(true);

    SECTION("WAMR")
    {
        faasmConf.wasmVm = "wamr";
    }

    faabric::Message result =
      executeWithPool(req, OMP_TEST_TIMEOUT_MS, false).at(0);

    // Get result
    REQUIRE(result.returnvalue() > 0);
}
}
