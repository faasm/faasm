#include <catch/catch.hpp>

#include <infra/infra.h>

using namespace infra;

namespace tests {

    TEST_CASE("Test simple kv retrieval", "[state]") {
        State s;
        std::string newKey = "test_state_new";

        StateKeyValue *kv = s.getKV(newKey);

        const std::vector<uint8_t> &actual = kv->get();
        REQUIRE(actual.empty());
    }
}