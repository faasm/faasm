#include <catch2/catch.hpp>

#include "utils.h"

#include <faabric/util/func.h>

namespace tests {

void doOmpTest(const std::string& function)
{
    cleanSystem();

    faabric::util::SystemConfig& conf = faabric::util::getSystemConfig();
    std::string& originalThreadMode = conf.threadMode;

    // Set up local OMP
    conf.threadMode = "local";

    faabric::Message msg = faabric::util::messageFactory("omp", function);
    execFunction(msg);

    // Reset config
    conf.threadMode = originalThreadMode;
}

TEST_CASE("Test static for scheduling", "[wasm][openmp]")
{
    doOmpTest("for_static_schedule");
}

TEST_CASE("Test OMP header API functions", "[wasm][openmp]")
{
    doOmpTest("header_api_support");
}

TEST_CASE("Test running OpenMP checks", "[wasm][openmp]")
{
    doOmpTest("omp_checks");
}

TEST_CASE("Test non-nested barrier pragma", "[wasm][openmp]")
{
    doOmpTest("simple_barrier");
}

TEST_CASE("Test basic omp parallel for pragma", "[wasm][openmp]")
{
    doOmpTest("simple_for");
}

TEST_CASE("Test non-nested master pragma", "[wasm][openmp]")
{
    doOmpTest("simple_master");
}

TEST_CASE("Test thread stack mapping", "[wasm][openmp]")
{
    doOmpTest("stack_debug");
}

TEST_CASE("Test simple reduction function", "[wasm][openmp]")
{
    doOmpTest("simple_reduce");
}

TEST_CASE("Test averaging with different methods (atomic RR and reduction)",
          "[wasm][openmp]")
{
    doOmpTest("reduction_average");
}

TEST_CASE("Test integrating using many OpenMP constructs", "[wasm][openmp]")
{
    doOmpTest("reduction_integral");
}

TEST_CASE("Test critical section", "[wasm][openmp]")
{
    doOmpTest("simple_critical");
}

TEST_CASE("Test single section", "[wasm][openmp]")
{
    doOmpTest("simple_single");
}

TEST_CASE("Test custom reduction function", "[wasm][openmp]")
{
    doOmpTest("custom_reduce");
}

TEST_CASE("Test nested API", "[wasm][openmp]")
{
    doOmpTest("nested_levels_test");
}

TEST_CASE("Test nested parallel region support", "[wasm][openmp]")
{
    doOmpTest("nested_parallel");
}

TEST_CASE("Test openmp Pi calculation", "[wasm][openmp]")
{
    doOmpTest("mt_pi");
}

TEST_CASE("Test proper handling of getting and setting next level num threads",
          "[wasm][openmp]")
{
    doOmpTest("setting_num_threads");
}
}
