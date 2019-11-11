#include <catch/catch.hpp>

#include <util/config.h>
#include <emulator/emulator.h>
#include <faasm/core.h>
#include <util/func.h>

namespace tests {
    TEST_CASE("Test invoking function with emulator chaining", "[knative]") {
        const message::Message call = util::messageFactory("demo", "chain");
        setEmulatedMessage(call);

        std::vector<uint8_t> inputOne = {1, 2, 3};
        std::vector<uint8_t> inputTwo = {2, 3, 4};

        unsigned int callIdOne = faasmChainThisInput(1, inputOne.data(), 3);
        unsigned int callIdTwo = faasmChainThisInput(2, inputTwo.data(), 3);

        unsigned int resultOne = faasmAwaitCall(callIdOne);
        unsigned int resultTwo = faasmAwaitCall(callIdTwo);

        REQUIRE(resultOne == 0);
        REQUIRE(resultTwo == 0);
    }
}