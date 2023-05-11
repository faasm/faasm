#include <catch2/catch.hpp>

#include "faasm_fixtures.h"
#include "utils.h"

#include <boost/filesystem/operations.hpp>
#include <faabric/util/func.h>

namespace tests {
// NOTE: if this is failing make sure you've set up libfake
TEST_CASE_METHOD(FunctionExecTestFixture,
                 "Test e2e execution of dynamic linking modules",
                 "[faaslet]")
{
    const char* filePath =
      "/usr/local/faasm/runtime_root/lib/fake/libfakeLibA.so";
    REQUIRE(boost::filesystem::exists(filePath));

    auto req = setUpContext("demo", "dynlink");

    SECTION("Single execution") { executeWithPool(req); }

    SECTION("Multiple execution") { executeWithPoolMultipleTimes(req, 3); }
}
}
