#include <catch/catch.hpp>
#include <util/bytes.h>
#include <util/queue.h>

using namespace util;

typedef util::Queue<int> IntQueue;

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