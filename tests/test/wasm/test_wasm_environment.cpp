#include "utils.h"
#include "wasm/WasmEnvironment.h"
#include <catch2/catch.hpp>

#include <cstdlib>

#include <faabric/util/bytes.h>
#include <faabric/util/config.h>
#include <faabric/util/func.h>

namespace tests {

TEST_CASE("Test wasm environment", "[wasm]")
{
    const char* originalLcTypes = getenv("LC_CTYPE");

    std::string expected;
    SECTION("Default values")
    {
        unsetenv("LC_CTYPE");
        expected = "en_GB.UTF-8";
    }

    SECTION("Environment values")
    {
        setenv("LC_CTYPE", "foobar", 1);
        expected = "foobar";
    }

    wasm::WasmEnvironment env;
    REQUIRE(env.getEnv("LC_CTYPE") == expected);

    // Reset the environment
    if (originalLcTypes == nullptr) {
        unsetenv("LC_CTYPE");
    } else {
        setenv("LC_CTYPE", originalLcTypes, 1);
    }
}
}
