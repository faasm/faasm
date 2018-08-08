#include <catch/catch.hpp>
#include <proto/faasm.pb.h>
#include <iostream>

namespace tests {

    std::vector<uint8_t> createInput(int start, int length) {
        std::vector<uint8_t> result;
        result.reserve((unsigned long) length);

        for (int i = start; i < start + length; i++) {
            result.emplace_back((uint8_t) i);
        }

        return result;
    }

    void checkBytesVsString(const std::vector<uint8_t> &bytes,const std::string &str) {
        const char *cstr = str.c_str();

        auto *rawBytes = reinterpret_cast<const uint8_t*>(cstr);

        std::vector<uint8_t> actual(rawBytes, rawBytes + str.length());

        REQUIRE(bytes == actual);
    }

    TEST_CASE("Test protobuf classes", "[proto]") {
        message::FunctionCall funcCall;

        std::string user = "foobar user";
        std::string func = "foobar func";
        std::string resultKey = "dummy result";

        std::vector<uint8_t> inputData = createInput(0, 100);
        std::vector<uint8_t> outputData = createInput(120, 50);

        funcCall.set_user(user);
        funcCall.set_function(func);
        funcCall.set_resultkey(resultKey);

        funcCall.set_inputdata(inputData.data(), 100);
        funcCall.set_outputdata(outputData.data(), 50);

        REQUIRE(user == funcCall.user());
        REQUIRE(func == funcCall.function());
        REQUIRE(resultKey == funcCall.resultkey());

        // Check serialisation round trip
        std::string serialised = funcCall.SerializeAsString();

        message::FunctionCall newFuncCall;
        newFuncCall.ParseFromString(serialised);

        REQUIRE(user == newFuncCall.user());
        REQUIRE(func == newFuncCall.function());
        REQUIRE(resultKey == newFuncCall.resultkey());

        // Check input/ output data
        const std::string actualInput = newFuncCall.inputdata();
        const std::string actualOutput = newFuncCall.outputdata();

        checkBytesVsString(inputData, actualInput);
        checkBytesVsString(outputData, actualOutput);
    }
}