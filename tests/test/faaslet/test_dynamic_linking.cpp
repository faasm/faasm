#include <catch2/catch.hpp>

#include "utils.h"

#include <boost/filesystem/operations.hpp>
#include <faabric/util/func.h>

namespace tests {
// NOTE: if this is failing make sure you've set up libfake
TEST_CASE("Test e2e execution of dynamic linking modules", "[faaslet]")
{
    cleanSystem();

    const char* filePath =
      "/usr/local/faasm/runtime_root/lib/fake/libfakeLibA.so";
    REQUIRE(boost::filesystem::exists(filePath));

    faabric::Message msg = faabric::util::messageFactory("demo", "dynlink");

    SECTION("Single execution") { execFunction(msg); }

    SECTION("Multiple execution") { checkMultipleExecutions(msg, 3); }
}
}
