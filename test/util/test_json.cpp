#include <catch/catch.hpp>

#include "utils.h"

#include <util/json.h>

using namespace util;

namespace tests {
    TEST_CASE("Test message to JSON round trip", "[util]") {
        message::Message msg;
        msg.set_id(1234465);
        msg.set_user("user 1");
        msg.set_function("great function");
        msg.set_idx(50);
        msg.set_hops(34);

        msg.set_pythonuser("py user");
        msg.set_pythonfunction("py func");
        msg.set_pythonidx(12);

        msg.set_isasync(true);
        msg.set_ispython(true);
        msg.set_istypescript(true);
        msg.set_isstatusrequest(true);
        msg.set_isflushrequest(true);

        msg.set_resultkey("this is a result");
        msg.set_statuskey("this is a status");

        msg.set_coldstartinterval(4);

        msg.set_ismpi(true);
        msg.set_mpiworldid(1234);
        msg.set_mpirank(5678);
        msg.set_mpiworldsize(33);

        msg.set_cmdline("some cmdline");

        SECTION("Dodgy characters") {
            msg.set_inputdata("[0], %$ 2233 9");
        }
        SECTION("Bytes") {
            std::vector<uint8_t> bytes = {0, 0, 1, 1, 0, 2, 2, 3, 3, 4, 4};
            msg.set_inputdata(bytes.data(), bytes.size());
        }

        util::setMessageId(msg);

        std::string jsonString = util::messageToJson(msg);

        message::Message actual = util::jsonToMessage(jsonString);

        checkMessageEquality(msg, actual);
    }

    TEST_CASE("Test get JSON property from JSON string", "[util]") {
        // Valid lookups
        REQUIRE(getValueFromJsonString("foo", "{\"foo\": \"bar\"}") == "bar");
        REQUIRE(getValueFromJsonString("foo", "{\"foo\": \"bar\", \"other\": \"value\"}") == "bar");
        REQUIRE(getValueFromJsonString("foo", "{\"other\": \"value\", \"foo\": \"abc\"}") == "abc");

        // Nothing to return
        REQUIRE(getValueFromJsonString("foo", "{\"FOO\": \"aa\", \"blah\": \"xxx\"}") == "");
        REQUIRE(getValueFromJsonString("foo", "{\"foo\": \"\"}") == "");
    }
}