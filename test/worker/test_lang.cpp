#include <catch/catch.hpp>

#include "utils.h"

#include <util/func.h>

namespace tests {
    TEST_CASE("Test pointer to poitner", "[worker]") {
        cleanSystem();

        message::Message msg;
        msg.set_user("demo");
        msg.set_function("ptr_ptr");
        msg.set_resultkey("ptr_ptr_test");

        execFunction(msg);
    }
}