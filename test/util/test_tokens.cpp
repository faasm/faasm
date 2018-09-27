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

        std::vector<int> acquiredTokens;
    }

    void getTokenTask() {
        // Get a token and add to the list of acquired tokens
        int token = vars::sharedPool.getToken();
        vars::acquiredTokens.push_back(token);
    }

    TEST_CASE("Test multithreaded token pool operation", "[util]") {
        // Set off threads to get tokens
        std::thread t1(getTokenTask);
        std::thread t2(getTokenTask);
        std::thread t3(getTokenTask);

        t1.join();
        t2.join();
        t3.join();

        REQUIRE(vars::acquiredTokens.size() == 3);
        std::vector<int> expected = {0, 1, 2};
        REQUIRE(vars::acquiredTokens == expected);

        // Start a blocking thread in the background
        std::thread t4(getTokenTask);
        t4.detach();

        // TODO sleeping here to try and get another thread to execute is sketchy
        std::this_thread::sleep_for(std::chrono::milliseconds(300));

        // Check the acquired tokens hasn't changed
        REQUIRE(vars::acquiredTokens == expected);

        // Now release a token
        vars::sharedPool.releaseToken(1);

        // TODO: sketchy again
        std::this_thread::sleep_for(std::chrono::milliseconds(300));

        // Check token has been acquired
        std::vector<int> expected2 = {0, 1, 2, 1};
        REQUIRE(vars::acquiredTokens.size() == expected2.size());
        REQUIRE(vars::acquiredTokens == expected2);
    }
}