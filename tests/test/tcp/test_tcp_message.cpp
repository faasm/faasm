#include <catch/catch.hpp>

#include <tcp/TCPMessage.h>

using namespace tcp;

namespace tests {
    TEST_CASE("Test message null buffer", "[tcp]") {
        TCPMessage m {
            .type = TCPMessageType::STANDARD,
            .len = 0
        };

        uint8_t *buffer = tcpMessageToBuffer(&m);

        TCPMessage *m2 = tcpMessageFromBuffer(buffer);
        REQUIRE(m2->type == TCPMessageType::STANDARD);
        REQUIRE(m2->len == 0);
    }

    TEST_CASE("Test message with buffer", "[tcp]") {
        std::vector<uint8_t> bytes = {0, 1, 2, 3, 4};
        TCPMessage m {
            .type = TCPMessageType::STANDARD,
            .len = bytes.size(),
            .buffer = bytes.data()
        };

        uint8_t *buffer = tcpMessageToBuffer(&m);

        TCPMessage *m2 = tcpMessageFromBuffer(buffer);
        REQUIRE(m2->type == TCPMessageType::STANDARD);
        REQUIRE(m2->len == bytes.size());

        std::vector<uint8_t> actual(m2->buffer, m2->buffer + m2->len);
        REQUIRE(actual == bytes);
    }
}