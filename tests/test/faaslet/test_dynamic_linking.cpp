#include <catch/catch.hpp>

#include "utils.h"

#include <util/func.h>
#include <boost/filesystem/operations.hpp>

namespace tests {
    // NOTE: if this is failing make sure you've set up libfake
    TEST_CASE("Test e2e execution of dynamic linking modules", "[faaslet]") {
        cleanSystem();

        const char *filePath = "/usr/local/faasm/runtime_root/lib/libfakeLibA.so";
        REQUIRE(boost::filesystem::exists(filePath));

        message::Message msg = util::messageFactory("demo", "dynlink");

        SECTION("Single execution") {
            execFunction(msg);
        }

        SECTION("Multiple execution") {
            checkMultipleExecutions(msg, 3);
        }
    }
}