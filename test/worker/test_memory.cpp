#include <catch/catch.hpp>

#include "utils.h"

#include <util/func.h>

namespace tests {
    TEST_CASE("Test memcpy", "[worker]") {
        cleanSystem();

        message::Message msg;
        msg.set_user("demo");
        msg.set_function("memcpy");
        msg.set_resultkey("memcpy_test");

        execFunction(msg);
    }

    TEST_CASE("Test memmove", "[worker]") {
        cleanSystem();

        message::Message msg;
        msg.set_user("demo");
        msg.set_function("memmove");
        msg.set_resultkey("memmove_test");

        execFunction(msg);
    }

    TEST_CASE("Test sbrk", "[worker]") {
        cleanSystem();

        message::Message msg;
        msg.set_user("demo");
        msg.set_function("sbrk");
        msg.set_resultkey("sbrk_test");

        execFunction(msg);
    }
}