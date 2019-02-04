#include <catch/catch.hpp>

#include "utils.h"

using namespace faasm;

namespace tests {
    void checkMessageEquality(const message::Message &msgA, const message::Message &msgB) {
        REQUIRE(msgA.user() == msgB.user());
        REQUIRE(msgA.function() == msgB.function());
        REQUIRE(msgA.isasync() == msgB.isasync());
        REQUIRE(msgA.success() == msgB.success());
        REQUIRE(msgA.inputdata() == msgB.inputdata());
        REQUIRE(msgA.outputdata() == msgB.outputdata());
        REQUIRE(msgA.resultkey() == msgB.resultkey());
        REQUIRE(msgA.type() == msgB.type());
    }
}

