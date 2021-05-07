#include <catch2/catch.hpp>

#include "utils.h"

#include <faabric/snapshot/SnapshotRegistry.h>
#include <faabric/util/config.h>
#include <faabric/util/environment.h>
#include <faabric/util/func.h>

// Longer timeout to allow longer-running functions to finish
#define OMP_TEST_TIMEOUT_MS 10000

namespace tests {

void doOmpTestLocal(const std::string& function)
{
    cleanSystem();

    // Make sure we have enough thread pool capacity
    faabric::util::SystemConfig& conf = faabric::util::getSystemConfig();
    int32_t initialCpu = conf.overrideCpuCount;
    conf.overrideCpuCount = 15;

    faabric::Message msg = faabric::util::messageFactory("omp", function);
    execFuncWithPool(msg, false, OMP_TEST_TIMEOUT_MS);

    conf.overrideCpuCount = initialCpu;
}

TEST_CASE("Test static for scheduling", "[wasm][openmp]")
{
    doOmpTestLocal("for_static_schedule");
}

TEST_CASE("Test OMP header API functions", "[wasm][openmp]")
{
    doOmpTestLocal("header_api_support");
}

TEST_CASE("Test running OpenMP checks", "[wasm][openmp]")
{
    doOmpTestLocal("omp_checks");
}

TEST_CASE("Test non-nested barrier pragma", "[wasm][openmp]")
{
    doOmpTestLocal("simple_barrier");
}

TEST_CASE("Test basic omp parallel for pragma", "[wasm][openmp]")
{
    doOmpTestLocal("simple_for");
}

TEST_CASE("Test non-nested master pragma", "[wasm][openmp]")
{
    doOmpTestLocal("simple_master");
}

TEST_CASE("Test simple reduction function", "[wasm][openmp]")
{
    doOmpTestLocal("simple_reduce");
}

TEST_CASE("Test integrating using different constructs", "[wasm][openmp]")
{
    doOmpTestLocal("reduction_integral");
}

TEST_CASE("Test critical section", "[wasm][openmp]")
{
    doOmpTestLocal("simple_critical");
}

TEST_CASE("Test single section", "[wasm][openmp]")
{
    doOmpTestLocal("simple_single");
}

TEST_CASE("Test custom reduction function", "[wasm][openmp]")
{
    doOmpTestLocal("custom_reduce");
}

TEST_CASE("Test nested API", "[wasm][openmp]")
{
    doOmpTestLocal("nested_levels_test");
}

TEST_CASE("Test nested parallel region support", "[wasm][openmp]")
{
    doOmpTestLocal("nested_parallel");
}

TEST_CASE("Test openmp Pi calculation", "[wasm][openmp]")
{
    doOmpTestLocal("mt_pi");
}

TEST_CASE("Test getting and setting num threads", "[wasm][openmp]")
{
    doOmpTestLocal("setting_num_threads");
}

TEST_CASE("Test openmp wtime", "[wasm][openmp]")
{
    doOmpTestLocal("wtime");
}

TEST_CASE("Test single-threaded reduction", "[wasm][openmp]")
{
    doOmpTestLocal("single_thread_reduce");
}

TEST_CASE("Test more complex reduction", "[wasm][openmp]")
{
    doOmpTestLocal("complex_reduce");
}

TEST_CASE("Run openmp memory stress test", "[wasm][openmp]")
{
    cleanSystem();

    // Overload the number of cores
    int nCores = 50;
    faabric::Message msg = faabric::util::messageFactory("omp", "mem_stress");
    msg.set_cmdline(std::to_string(nCores));

    execFunction(msg);
}
}
