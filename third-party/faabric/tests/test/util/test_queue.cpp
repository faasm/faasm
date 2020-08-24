#include <catch/catch.hpp>
#include <faabric/util/bytes.h>
#include <faabric/util/queue.h>

using namespace faabric::util;

typedef faabric::utilQueue<int> IntQueue;

namespace tests {
    TEST_CASE("Test queue operations", "[util]") {
        IntQueue q;

        q.enqueue(1);
        q.enqueue(2);
        q.enqueue(3);
        q.enqueue(4);
        q.enqueue(5);

        REQUIRE(q.dequeue() == 1);
        REQUIRE(q.dequeue() == 2);
        REQUIRE(q.peek() == 3);
        REQUIRE(q.peek() == 3);
        REQUIRE(q.peek() == 3);
        REQUIRE(q.dequeue() == 3);
        REQUIRE(q.dequeue() == 4);
        REQUIRE(q.dequeue() == 5);

        REQUIRE_THROWS(q.dequeue(1));
    }
}