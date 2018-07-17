#include <catch/catch.hpp>
#include <proto/faasm.pb.h>
#include <iostream>

namespace tests {

    TEST_CASE("Test protobuf classes", "[proto]") {
        message::FunctionCall funcCall;

        std::string user = "foobar user";
        std::string func = "foobar func";

        funcCall.set_user(user);
        funcCall.set_function(func);

        REQUIRE(user == funcCall.user());
        REQUIRE(func == funcCall.function());

        // Check serialisation round trip
        std::string serialised = funcCall.SerializeAsString();

        message::FunctionCall newFuncCall;
        newFuncCall.ParseFromString(serialised);

        REQUIRE(user == newFuncCall.user());
        REQUIRE(func == newFuncCall.function());
    }
}