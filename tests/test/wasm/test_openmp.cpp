#include <catch2/catch.hpp>

#include "conf/FaasmConfig.h"
#include "faabric/util/environment.h"
#include "utils.h"

#include <faabric/snapshot/SnapshotRegistry.h>
#include <faabric/util/func.h>

namespace tests {

void doOmpTestLocal(const std::string& function)
{
    cleanSystem();

    faabric::snapshot::SnapshotRegistry& reg =
      faabric::snapshot::getSnapshotRegistry();
    REQUIRE(reg.getSnapshotCount() == 0);

    // Make sure we have enough thread pool capacity
    conf::FaasmConfig& conf = conf::getFaasmConfig();
    int32_t defaultPoolSize = conf.moduleThreadPoolSize;
    conf.moduleThreadPoolSize = 15;

    faabric::Message msg = faabric::util::messageFactory("omp", function);
    execFunction(msg);

    // Check only the main snapshot exists
    REQUIRE(reg.getSnapshotCount() == 1);

    conf.moduleThreadPoolSize = defaultPoolSize;
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

TEST_CASE("Test averaging with different methods (atomic RR and reduction)",
          "[wasm][openmp]")
{
    doOmpTestLocal("reduction_average");
}

TEST_CASE("Test integrating using many OpenMP constructs", "[wasm][openmp]")
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

TEST_CASE("Test proper handling of getting and setting next level num threads",
          "[wasm][openmp]")
{
    doOmpTestLocal("setting_num_threads");
}

TEST_CASE("Test openmp wtime", "[wasm][openmp]")
{
    doOmpTestLocal("wtime");
}

TEST_CASE("Run openmp memory stress test", "[wasm][openmp]")
{
    // Overload the number of cores, and run several times to make sure we
    // stress enough to flush out errors
    int nCores = 10 * faabric::util::getUsableCores();
    faabric::Message msg = faabric::util::messageFactory("omp", "mem_stress");
    msg.set_cmdline(std::to_string(nCores));

    for (int i = 0; i < 5; i++) {
        cleanSystem();
        execFunction(msg);
    }
}
}
