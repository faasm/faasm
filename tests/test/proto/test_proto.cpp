#include <catch/catch.hpp>
#include <iostream>

#include <proto/faasm.pb.h>
#include <util/bytes.h>
#include <utils.h>

namespace tests {

    std::vector<uint8_t> createInput(int start, int length) {
        std::vector<uint8_t> result;
        result.reserve((unsigned long) length);

        for (int i = start; i < start + length; i++) {
            result.emplace_back((uint8_t) i);
        }

        return result;
    }

    TEST_CASE("Test protobuf classes", "[proto]") {
        message::Message funcCall;

        std::string user = "foobar user";
        std::string func = "foobar func";
        std::string resultKey = "dummy result";
        int returnValue = 123;

        std::string pyUser = "python user";
        std::string pyFunc = "python func";
        int pyIdx = 11;

        std::vector<uint8_t> inputData = createInput(0, 100);
        std::vector<uint8_t> outputData = createInput(120, 50);

        std::string cmdline = "some cmdline args";

        funcCall.set_user(user);
        funcCall.set_function(func);
        funcCall.set_resultkey(resultKey);
        funcCall.set_returnvalue(returnValue);

        funcCall.set_pythonuser(pyUser);
        funcCall.set_pythonfunction(pyFunc);
        funcCall.set_pythonidx(pyIdx);

        funcCall.set_inputdata(inputData.data(), 100);
        funcCall.set_outputdata(outputData.data(), 50);

        funcCall.set_isasync(true);
        funcCall.set_ispython(true);
        funcCall.set_istypescript(true);
        funcCall.set_isstatusrequest(true);
        funcCall.set_isflushrequest(true);

        funcCall.set_type(message::Message_MessageType_BIND);

        funcCall.set_cmdline(cmdline);

        REQUIRE(funcCall.type() == message::Message_MessageType_BIND);
        REQUIRE(user == funcCall.user());
        REQUIRE(func == funcCall.function());
        REQUIRE(resultKey == funcCall.resultkey());
        REQUIRE(returnValue == funcCall.returnvalue());

        // Check serialisation round trip
        std::string serialised = funcCall.SerializeAsString();

        message::Message newFuncCall;
        newFuncCall.ParseFromString(serialised);

        REQUIRE(user == newFuncCall.user());
        REQUIRE(func == newFuncCall.function());
        REQUIRE(resultKey == newFuncCall.resultkey());
        REQUIRE(message::Message_MessageType_BIND == newFuncCall.type());

        REQUIRE(pyUser == newFuncCall.pythonuser());
        REQUIRE(pyFunc == newFuncCall.pythonfunction());
        REQUIRE(pyIdx == newFuncCall.pythonidx());

        REQUIRE(newFuncCall.isasync());
        REQUIRE(newFuncCall.ispython());
        REQUIRE(newFuncCall.istypescript());
        REQUIRE(newFuncCall.isstatusrequest());
        REQUIRE(newFuncCall.isflushrequest());

        REQUIRE(cmdline == newFuncCall.cmdline());

        // Check input/ output data
        const std::string actualStrInput = newFuncCall.inputdata();
        const std::string actualStrOutput = newFuncCall.outputdata();

        const std::vector<uint8_t> actualBytesInput = util::stringToBytes(actualStrInput);
        const std::vector<uint8_t> actualBytesOutput = util::stringToBytes(actualStrOutput);

        REQUIRE(inputData == actualBytesInput);
        REQUIRE(outputData == actualBytesOutput);
    }

    TEST_CASE("Test protobuf byte handling", "[proto]") {
        // One message with null terminators, one without
        message::Message msgA;
        std::vector<uint8_t> bytesA = {0, 0, 1, 1, 0, 0, 2, 2};
        msgA.set_inputdata(bytesA.data(), bytesA.size());

        message::Message msgB;
        std::vector<uint8_t> bytesB = {1, 1, 1, 1, 1, 1, 2, 2};
        msgB.set_inputdata(bytesB.data(), bytesB.size());

        std::string serialisedA = msgA.SerializeAsString();
        std::string serialisedB = msgB.SerializeAsString();

        REQUIRE(serialisedA.size() == serialisedB.size());

        message::Message newMsgA;
        newMsgA.ParseFromString(serialisedA);

        message::Message newMsgB;
        newMsgB.ParseFromString(serialisedB);

        checkMessageEquality(msgA, newMsgA);
        checkMessageEquality(msgB, newMsgB);
    }
}