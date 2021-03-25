#include <catch2/catch.hpp>

#include "utils.h"

#include <faabric/util/func.h>

namespace tests {
void checkThreadedFunction(const char* threadFunc, bool runPool)
{
    cleanSystem();

    // Run the function
    faabric::Message msg = faabric::util::messageFactory("demo", threadFunc);

    if (runPool) {
        execFuncWithPool(msg, false, 1, false, 4, false);
    } else {
        execFunction(msg);
    }
}

TEST_CASE("Test local-only threading", "[faaslet]")
{
    checkThreadedFunction("threads_local", false);
}

TEST_CASE("Run thread checks locally", "[faaslet]")
{
    checkThreadedFunction("threads_check", false);
}

TEST_CASE("Run thread checks with chaining", "[faaslet]")
{
    checkThreadedFunction("threads_check", true);
}

TEST_CASE("Run distributed threading check", "[faaslet]")
{
    checkThreadedFunction("threads_dist", true);
}
}
