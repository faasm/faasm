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

    std::string doOmpTestLocal(const std::string& function, int numThreads = -1)
    {
        auto req = faabric::util::batchExecFactory("omp", function, 1);
        req->set_singlehosthint(true);
        if (numThreads > 0) {
            req->mutable_messages(0)->set_isomp(true);
            req->mutable_messages(0)->set_ompnumthreads(numThreads);
            req->mutable_messages(0)->set_inputdata(std::to_string(numThreads));
        }

        // 08/03/2024 - Some local OpenMP tests stopped working with WAVM
        // after upgrade to LLVM 17 due to WAVM's incorrect handling of
        // atomics
        // SECTION("WAVM") { faasmConf.wasmVm = "wavm"; }

        faasmConf.wasmVm = "wamr";

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

    int nThreads = 0;

    SECTION("Single-threaded")
    {
        nThreads = 1;
    }

    SECTION("Multi-threaded")
    {
        nThreads = 10;
    }

    doOmpTestLocal("repeated_reduce", nThreads);
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

TEST_CASE_METHOD(OpenMPTestFixture,
                 "Test elastically scaling OpenMP execution (single-threaded)",
                 "[wasm][openmp]")
{
    int numThreads = 1;
    int numSlots = 10;
    int numLoops = 10;

    faabric::HostResources res;
    res.set_slots(numSlots);
    sch.setThisHostResources(res);
    faasmConf.wasmVm = "wamr";

    // The function does 10 loops, and we invoke it with 5 threads, so in
    // total we should have (5 - 1) * 10 + 1 = 41 message results. This is
    // because we fork with (n - 1) threads, and persist the main thread.
    // If we preload one scheduling decision, and then scale up, we should have
    // 4 + (10 - 1) * 9 + 1 = 86 message results
    auto req = faabric::util::batchExecFactory("omp", "repeated_reduce", 1);
    req->set_singlehosthint(true);
    req->mutable_messages(0)->set_isomp(true);
    req->mutable_messages(0)->set_ompnumthreads(numThreads);
    req->mutable_messages(0)->set_inputdata(std::to_string(numThreads));
    int expectedMessageresults = 0;
    std::shared_ptr<faabric::batch_scheduler::SchedulingDecision> preloadDec =
      nullptr;

    SECTION("Elastically scale")
    {
        req->set_elasticscalehint(true);

        // Prepare pre-loaded scheduling decision
        /*
        preloadDec = std::make_shared<batch_scheduler::SchedulingDecision>(
          req->appid(), req->groupid());
        for (int i = 0; i < numThreads; i++) {
            preloadDec->addMessage(conf.endpointHost, 0, 0, i);
        }
        */

        expectedMessageresults =
          (numThreads - 1) + (numSlots - 1) * (numLoops - 1) + 1;
    }

    SECTION("Do not elastically scale")
    {
        req->set_elasticscalehint(false);
        expectedMessageresults = (numThreads - 1) * numLoops + 1;
    }

    /*
    if (preloadDec != nullptr) {
        // Pre-load scheduling decision so that we do not elastically scale
        // the first iteration
        plannerCli.preloadSchedulingDecision(preloadDec);
    }
    */

    executeWithPool(req, OMP_TEST_TIMEOUT_MS, false);
    auto results = waitForBatchResults(req, expectedMessageresults);

    REQUIRE(results->messageresults().at(0).returnvalue() == 0);
    REQUIRE(results->messageresults_size() == expectedMessageresults);
}

TEST_CASE_METHOD(OpenMPTestFixture,
                 "Test elastically scaling OpenMP execution",
                 "[wasm][openmp]")
{
    int numThreads = 5;
    int numSlots = 10;
    int numLoops = 10;

    faabric::HostResources res;
    res.set_slots(numSlots);
    sch.setThisHostResources(res);
    faasmConf.wasmVm = "wamr";

    // The function does 10 loops, and we invoke it with 5 threads, so in
    // total we should have (5 - 1) * 10 + 1 = 41 message results. This is
    // because we fork with (n - 1) threads, and persist the main thread.
    // If we preload one scheduling decision, and then scale up, we should have
    // 4 + (10 - 1) * 9 + 1 = 86 message results
    auto req = faabric::util::batchExecFactory("omp", "repeated_reduce", 1);
    req->set_singlehosthint(true);
    req->mutable_messages(0)->set_isomp(true);
    req->mutable_messages(0)->set_ompnumthreads(numThreads);
    req->mutable_messages(0)->set_inputdata(std::to_string(numThreads));
    int expectedMessageresults = 0;
    std::shared_ptr<faabric::batch_scheduler::SchedulingDecision> preloadDec =
      nullptr;

    SECTION("Elastically scale")
    {
        req->set_elasticscalehint(true);

        // Prepare pre-loaded scheduling decision
        /*
        preloadDec = std::make_shared<batch_scheduler::SchedulingDecision>(
          req->appid(), req->groupid());
        for (int i = 0; i < numThreads; i++) {
            preloadDec->addMessage(conf.endpointHost, 0, 0, i);
        }
        */

        expectedMessageresults =
          (numThreads - 1) + (numSlots - 1) * (numLoops - 1) + 1;
    }

    SECTION("Do not elastically scale")
    {
        req->set_elasticscalehint(false);
        expectedMessageresults = (numThreads - 1) * numLoops + 1;
    }

    /*
    if (preloadDec != nullptr) {
        // Pre-load scheduling decision so that we do not elastically scale
        // the first iteration
        plannerCli.preloadSchedulingDecision(preloadDec);
    }
    */

    executeWithPool(req, OMP_TEST_TIMEOUT_MS, false);
    auto results = waitForBatchResults(req, expectedMessageresults);

    REQUIRE(results->messageresults().at(0).returnvalue() == 0);
    REQUIRE(results->messageresults_size() == expectedMessageresults);
}

TEST_CASE_METHOD(OpenMPTestFixture,
                 "Test concurrent OpenMP applications",
                 "[wasm][openmp]")
{
    int numThreadsA = 7;
    int numThreadsB = 3;
    int numSlots = 10;
    int numLoops = 10;

    faabric::HostResources res;
    res.set_slots(numSlots);
    sch.setThisHostResources(res);
    faasmConf.wasmVm = "wamr";

    auto reqA = faabric::util::batchExecFactory("omp", "repeated_reduce", 1);
    reqA->set_singlehosthint(true);
    reqA->mutable_messages(0)->set_inputdata(std::to_string(numThreadsA));

    auto reqB = faabric::util::batchExecFactory("omp", "repeated_reduce", 1);
    reqB->set_singlehosthint(true);
    reqB->mutable_messages(0)->set_inputdata(std::to_string(numThreadsB));

    int expectedMessageresultsA = (numThreadsA - 1) * numLoops + 1;
    int expectedMessageresultsB = (numThreadsB - 1) * numLoops + 1;

    auto& plannerCli = faabric::planner::getPlannerClient();
    plannerCli.callFunctions(reqA);
    plannerCli.callFunctions(reqB);

    auto resultsA = waitForBatchResults(reqA, expectedMessageresultsA);
    REQUIRE(resultsA->messageresults().at(0).returnvalue() == 0);
    REQUIRE(resultsA->messageresults_size() == expectedMessageresultsA);

    auto resultsB = waitForBatchResults(reqB, expectedMessageresultsB);
    REQUIRE(resultsB->messageresults().at(0).returnvalue() == 0);
    REQUIRE(resultsB->messageresults_size() == expectedMessageresultsB);
}

TEST_CASE_METHOD(OpenMPTestFixture,
                 "Test running out of slots throws exception",
                 "[wasm][openmp]")
{
// FIXME: the setjmp/longjmp mechanism is creating a TSAN race in this test
#if !(__has_feature(thread_sanitizer))
    faabric::HostResources res;
    res.set_slots(1);
    sch.setThisHostResources(res);

    faabric::Message msg =
      faabric::util::messageFactory("omp", "custom_reduce");
    auto req = faabric::util::batchExecFactory("omp", "custom_reduce", 1);
    req->set_singlehosthint(true);

    SECTION("WAMR")
    {
        faasmConf.wasmVm = "wamr";
    }

    faabric::Message result =
      executeWithPool(req, OMP_TEST_TIMEOUT_MS, false).at(0);

    REQUIRE(result.returnvalue() > 0);
#endif
}
}
