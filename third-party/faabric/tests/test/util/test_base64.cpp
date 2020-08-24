#include <catch/catch.hpp>

#include "faabric_utils.h"

#include <faabric/util/base64.h>
#include <faabric/util/func.h>

using namespace faabric::util;

namespace tests {
    std::string checkStringRoundTrip(const std::string &input) {
        const std::string encoded = b64encode(input);

        const std::string decoded = b64decode(encoded);

        REQUIRE(input == decoded);

        return decoded;
    }
    
    TEST_CASE("Test base64 encoding normal string", "[util]") {
        std::string input = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890#!£$%%^&()[]{}\n\t\r";
        checkStringRoundTrip(input);
    }

    TEST_CASE("Test base64 encoding protobuf object", "[util]") {
        faabric::Message msg;
        msg.set_user("user 1");
        msg.set_function("great function");
        msg.set_isasync(true);
        msg.set_ispython(true);
        msg.set_istypescript(true);
        msg.set_returnvalue(1234);
        msg.set_inputdata("[0], %$ 2233 9");
        msg.set_resultkey("blahblah");
        msg.set_type(faabric::Message_MessageType_BIND);

        const std::string &msgBytes = msg.SerializeAsString();

        const std::string &decoded = checkStringRoundTrip(msgBytes);
        faabric::Message actual;
        actual.ParseFromString(decoded);
        checkMessageEquality(msg, actual);
    }
}
