#include <catch/catch.hpp>
#include <util/queue.h>
#include <thread>

#include <iostream>

using namespace util;

namespace tests {
    TEST_CASE("Test basic token pool operation", "[util]") {
        TokenPool pool(5);

        // Remove some tokens from the pool
        REQUIRE(pool.getToken() == 0);
        REQUIRE(pool.getToken() == 1);
        REQUIRE(pool.getToken() == 2);

        // Put one back
        pool.releaseToken(1);

        // Get some more and check the one that's put back is reused
        REQUIRE(pool.getToken() == 3);
        REQUIRE(pool.getToken() == 4);
        REQUIRE(pool.getToken() == 1);
    }

    namespace vars {
        TokenPool sharedPool(3);

        std::mutex testMutex;
        std::vector<int> acquiredTokens;
    }

    void getTokenTask() {
        // Get a token and add to the list of acquired tokens
        int token = vars::sharedPool.getToken();

        std::scoped_lock<std::mutex> lock(vars::testMutex);
        vars::acquiredTokens.push_back(token);
    }

    TEST_CASE("Test token pool operation with some threads", "[util]") {
        std::vector<int> expected;
        REQUIRE(vars::acquiredTokens.empty());

        std::thread t1(getTokenTask);
        t1.join();
        expected.push_back(0);
        REQUIRE(vars::acquiredTokens == expected);

        std::thread t2(getTokenTask);
        t2.join();
        expected.push_back(1);
        REQUIRE(vars::acquiredTokens == expected);

        std::thread t3(getTokenTask);
        t3.join();
        expected.push_back(2);
        REQUIRE(vars::acquiredTokens == expected);

        vars::sharedPool.releaseToken(1);

        std::thread t4(getTokenTask);
        t4.join();
        expected.push_back(1);
        REQUIRE(vars::acquiredTokens == expected);
    }

    TEST_CASE("Test count functions", "[util]") {
        TokenPool p(10);
        REQUIRE(p.size() == 10);
        REQUIRE(p.free() == 10);
        REQUIRE(p.taken() == 0);

        int tokenA = p.getToken();
        int tokenB = p.getToken();

        REQUIRE(p.size() == 10);
        REQUIRE(p.free() == 8);
        REQUIRE(p.taken() == 2);

        p.releaseToken(tokenA);

        REQUIRE(p.size() == 10);
        REQUIRE(p.free() == 9);
        REQUIRE(p.taken() == 1);

        p.releaseToken(tokenB);

        REQUIRE(p.size() == 10);
        REQUIRE(p.free() == 10);
        REQUIRE(p.taken() == 0);
    }

    TEST_CASE("Test empty token pool returns -1", "[util]") {
        TokenPool p(0);

        int actualA = p.getToken();
        int actualB = p.getToken();
        int actualC = p.getToken();

        REQUIRE(actualA == -1);
        REQUIRE(actualB == -1);
        REQUIRE(actualC == -1);
    }
}