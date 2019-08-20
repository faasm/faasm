#include <catch/catch.hpp>

#include <thread>
#include "util/random.h"

using namespace util;

static std::vector<int> generated;

namespace tests {
    TEST_CASE("Test random string generation", "[util]") {
        std::string actualA = randomString(100);
        REQUIRE(actualA.size() == 100);

        std::string actualB = randomString(100);
        REQUIRE(actualB.size() == 100);

        REQUIRE(actualA != actualB);
    }

    TEST_CASE("Test multithreaded random number generation", "[util]") {

        // Generate random numbers in lots of threads
        std::vector<std::thread> threads(10);
        for(int i = 0; i < 10; i++) {
            threads.emplace_back([] {
                for(int j=0; j < 20; j++) {
                    generated.push_back(util::randomInteger(0, INT32_MAX));
                }
            });
        }

        // Check that there are no duplicates (if there's a problem there should reliably be several)



    }
}
