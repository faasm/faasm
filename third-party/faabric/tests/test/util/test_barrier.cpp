#include <catch/catch.hpp>
#include <faabric/util/bytes.h>
#include <faabric/util/barrier.h>
#include <thread>
#include <unistd.h>


using namespace faabric::util;

namespace tests {
    TEST_CASE("Test barrier operation", "[util]") {
        Barrier b(3);

        REQUIRE(b.getSlotCount() == 3);
        REQUIRE(b.getUseCount() == 0);

        auto t1 = std::thread([&b] {
            b.wait();
        });

        auto t2 = std::thread([&b] {
            b.wait();
        });

        // Sleep for a bit while the threads spawn
        usleep(500 * 1000);
        REQUIRE(b.getSlotCount() == 1);

        // Join with master to go through barrier
        b.wait();

        if (t1.joinable()) {
            t1.join();
        }

        if (t2.joinable()) {
            t2.join();
        }

        REQUIRE(b.getSlotCount() == 3);
        REQUIRE(b.getUseCount() == 1);
    }
}