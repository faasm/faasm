#include <catch/catch.hpp>

#include "utils.h"

#include <util/json.h>

using namespace util;

namespace tests {
    TEST_CASE("Test message to JSON round trip", "[util]") {
        message::Message msg;
        msg.set_user("user 1");
        msg.set_function("great function");

        msg.set_isasync(true);
        msg.set_ispython(true);

        msg.set_inputdata("[0], %$ 2233 9");
        msg.set_resultkey("blahblah");

        std::string jsonString = util::messageToJson(msg);

        message::Message actual = util::jsonToMessage(jsonString);

        checkMessageEquality(msg, actual);
    }
}