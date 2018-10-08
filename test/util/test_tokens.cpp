#include <catch/catch.hpp>
#include <util/util.h>
#include <thread>

#include <iostream>
#include <chrono>

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

        std::lock_guard<std::mutex> lock(vars::testMutex);
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

        //TODO: come up with a better test here to check blocking?
    }
}